#include <boost/algorithm/string/predicate.hpp>
#include <boost/beast.hpp>

#include "spdlog/spdlog.h"
#include <atomic>
#include <iostream>
#include <json.hpp>
#include <stdexcept>
#include <unistd.h>
#include <variant.hpp>

#include <mtx.hpp>
#include <mtx/identifiers.hpp>

#include "mtxclient/crypto/client.hpp"
#include "mtxclient/http/client.hpp"
#include "mtxclient/http/errors.hpp"

#include "mtxclient/utils.hpp"
#include "test_helpers.hpp"

//
// Simple example bot that will accept any invite.
//

using namespace std;
using namespace mtx::client;
using namespace mtx::crypto;
using namespace mtx::http;
using namespace mtx::events;
using namespace mtx::identifiers;

using TimelineEvent = mtx::events::collections::TimelineEvents;

constexpr auto OLM_ALGO = "m.olm.v1.curve25519-aes-sha2";

struct OlmCipherContent
{
        std::string body;
        uint8_t type;
};

inline void
from_json(const nlohmann::json &obj, OlmCipherContent &msg)
{
        msg.body = obj.at("body");
        msg.type = obj.at("type");
}

struct OlmMessage
{
        std::string sender_key;
        std::string sender;

        using RecipientKey = std::string;
        std::map<RecipientKey, OlmCipherContent> ciphertext;
};

inline void
from_json(const nlohmann::json &obj, OlmMessage &msg)
{
        if (obj.at("type") != "m.room.encrypted") {
                throw std::invalid_argument("invalid type for olm message");
        }

        if (obj.at("content").at("algorithm") != OLM_ALGO)
                throw std::invalid_argument("invalid algorithm for olm message");

        msg.sender     = obj.at("sender");
        msg.sender_key = obj.at("content").at("sender_key");
        msg.ciphertext =
          obj.at("content").at("ciphertext").get<std::map<std::string, OlmCipherContent>>();
}

template<class Container, class Item>
bool
exists(const Container &container, const Item &item)
{
        return container.find(item) != container.end();
}

void
get_device_keys(const UserId &user);

void
save_device_keys(const mtx::responses::QueryKeys &res);

void
mark_encrypted_room(const RoomId &id);

void
handle_to_device_msgs(const std::vector<nlohmann::json> &to_device);

//! Metadata associated with each active megolm session.
struct GroupSessionMsgData
{
        std::string session_id;
        std::string room_id;
        std::string event_id;

        uint64_t origin_server_ts;
        uint64_t message_index;
};

struct Storage
{
        //! Storage for the user_id -> list of devices mapping.
        std::map<std::string, std::vector<std::string>> devices_;
        //! Storage for the identity key for a device.
        std::map<std::string, std::string> device_keys_;
        //! Flag that indicate if a specific room has encryption enabled.
        std::map<std::string, bool> encrypted_rooms_;

        //! Mapping from curve25519 to session.
        std::map<std::string, OlmSessionPtr> olm_sessions;

        std::map<std::string, InboundGroupSessionPtr> inbound_group_sessions;

        bool inbound_group_exists(const std::string &room_id,
                                  const std::string &session_id,
                                  const std::string &sender_key)
        {
                const auto key = room_id + session_id + sender_key;
                return inbound_group_sessions.find(key) != inbound_group_sessions.end();
        }

        void set_inbound_group_session(const std::string &room_id,
                                       const std::string &session_id,
                                       const std::string &sender_key,
                                       InboundGroupSessionPtr session)
        {
                const auto key              = room_id + session_id + sender_key;
                inbound_group_sessions[key] = std::move(session);
        }

        OlmInboundGroupSession *get_inbound_group_session(const std::string &room_id,
                                                          const std::string &session_id,
                                                          const std::string &sender_key)
        {
                const auto key = room_id + session_id + sender_key;
                return inbound_group_sessions[key].get();
        }
};

namespace {
std::shared_ptr<Client> client        = nullptr;
std::shared_ptr<OlmClient> olm_client = nullptr;
Storage storage;

auto console = spdlog::stdout_color_mt("console");
}

void
print_errors(RequestErr err)
{
        if (err->status_code != boost::beast::http::status::unknown)
                console->error("status code: {}", static_cast<uint16_t>(err->status_code));
        if (!err->matrix_error.error.empty())
                console->error("matrix error: {}", err->matrix_error.error);
        if (err->error_code)
                console->error("error code: {}", err->error_code.message());
}

template<class T>
bool
is_room_encryption(const T &event)
{
        using namespace mtx::events;
        using namespace mtx::events::state;
        return mpark::holds_alternative<StateEvent<Encryption>>(event);
}

bool
is_encrypted(const TimelineEvent &event)
{
        using namespace mtx::events;
        return mpark::holds_alternative<EncryptedEvent<msg::Encrypted>>(event);
}

template<class T>
bool
is_member_event(const T &event)
{
        using namespace mtx::events;
        using namespace mtx::events::state;
        return mpark::holds_alternative<StateEvent<Member>>(event);
}

// Check if the given event has a textual representation.
bool
is_room_message(const TimelineEvent &event)
{
        return mpark::holds_alternative<mtx::events::RoomEvent<msg::Audio>>(event) ||
               mpark::holds_alternative<mtx::events::RoomEvent<msg::Emote>>(event) ||
               mpark::holds_alternative<mtx::events::RoomEvent<msg::File>>(event) ||
               mpark::holds_alternative<mtx::events::RoomEvent<msg::Image>>(event) ||
               mpark::holds_alternative<mtx::events::RoomEvent<msg::Notice>>(event) ||
               mpark::holds_alternative<mtx::events::RoomEvent<msg::Text>>(event) ||
               mpark::holds_alternative<mtx::events::RoomEvent<msg::Video>>(event);
}

// Retrieves the fallback body value from the event.
std::string
get_body(const TimelineEvent &event)
{
        if (mpark::holds_alternative<RoomEvent<msg::Audio>>(event))
                return mpark::get<RoomEvent<msg::Audio>>(event).content.body;
        else if (mpark::holds_alternative<RoomEvent<msg::Emote>>(event))
                return mpark::get<RoomEvent<msg::Emote>>(event).content.body;
        else if (mpark::holds_alternative<RoomEvent<msg::File>>(event))
                return mpark::get<RoomEvent<msg::File>>(event).content.body;
        else if (mpark::holds_alternative<RoomEvent<msg::Image>>(event))
                return mpark::get<RoomEvent<msg::Image>>(event).content.body;
        else if (mpark::holds_alternative<RoomEvent<msg::Notice>>(event))
                return mpark::get<RoomEvent<msg::Notice>>(event).content.body;
        else if (mpark::holds_alternative<RoomEvent<msg::Text>>(event))
                return mpark::get<RoomEvent<msg::Text>>(event).content.body;
        else if (mpark::holds_alternative<RoomEvent<msg::Video>>(event))
                return mpark::get<RoomEvent<msg::Video>>(event).content.body;

        return "";
}

// Retrieves the sender of the event.
std::string
get_sender(const TimelineEvent &event)
{
        return mpark::visit([](auto e) { return e.sender; }, event);
}

template<class T>
std::string
get_json(const T &event)
{
        return mpark::visit([](auto e) { return json(e).dump(2); }, event);
}

void
keys_uploaded_cb(const mtx::responses::UploadKeys &, RequestErr err)
{
        if (err) {
                print_errors(err);
                return;
        }

        console->info("keys uploaded");
}

void
mark_encrypted_room(const RoomId &id)
{
        console->info("encryption is enabled for room: {}", id.get());
        storage.encrypted_rooms_[id.get()] = true;
}

void
decrypt_olm_message(const OlmMessage &olm_msg)
{
        console->info("OLM message");
        console->info("sender: {}", olm_msg.sender);
        console->info("sender_key: {}", olm_msg.sender_key);

        const auto my_id_key = olm_client->identity_keys().curve25519;
        for (const auto &cipher : olm_msg.ciphertext) {
                if (cipher.first == my_id_key) {
                        const auto msg_body = cipher.second.body;
                        const auto msg_type = cipher.second.type;

                        console->info("the message is meant for us");
                        console->info("body: {}", msg_body);
                        console->info("type: {}", msg_type);

                        if (msg_type == 0) {
                                console->info("opening session with {}", olm_msg.sender);
                                auto inbound_session = olm_client->create_inbound_session(msg_body);

                                auto ok = matches_inbound_session_from(
                                  inbound_session.get(), olm_msg.sender_key, msg_body);

                                if (!ok) {
                                        console->error("session could not be established");

                                } else {
                                        auto output = olm_client->decrypt_message(
                                          inbound_session.get(), msg_type, msg_body);

                                        auto plaintext = json::parse(
                                          std::string((char *)output.data(), output.size()));
                                        console->info("decrypted message: \n {}",
                                                      plaintext.dump(2));

                                        storage.olm_sessions.emplace(olm_msg.sender_key,
                                                                     std::move(inbound_session));

                                        std::string room_id = plaintext.at("content").at("room_id");
                                        std::string session_id =
                                          plaintext.at("content").at("session_id");
                                        std::string session_key =
                                          plaintext.at("content").at("session_key");

                                        if (storage.inbound_group_exists(
                                              room_id, session_id, olm_msg.sender_key)) {
                                                console->warn("megolm session already exists");
                                        } else {
                                                auto megolm_session =
                                                  olm_client->init_inbound_group_session(
                                                    session_key);

                                                storage.set_inbound_group_session(
                                                  room_id,
                                                  session_id,
                                                  olm_msg.sender_key,
                                                  std::move(megolm_session));

                                                console->info("megolm_session saved");
                                        }
                                }
                        }
                }
        }
}

void
parse_messages(const mtx::responses::Sync &res)
{
        for (const auto &room : res.rooms.invite) {
                auto room_id = parse<Room>(room.first);

                console->info("joining room {}", room_id.to_string());
                client->join_room(room_id, [room_id](const nlohmann::json &, RequestErr e) {
                        if (e) {
                                print_errors(e);
                                console->error("failed to join room {}", room_id.to_string());
                                return;
                        }
                });
        }

        // Check if we have any new m.room_key messages (i.e starting a new megolm session)
        handle_to_device_msgs(res.to_device);

        // Check if the uploaded one time keys are enough
        for (const auto &device : res.device_one_time_keys_count) {
                if (device.second < 50) {
                        olm_client->generate_one_time_keys(50 - device.second);
                        // TODO: Mark keys as sent
                        client->upload_keys(olm_client->create_upload_keys_request(),
                                            &keys_uploaded_cb);
                }
        }

        for (const auto &room : res.rooms.join) {
                const std::string room_id = room.first;

                for (const auto &e : room.second.state.events) {
                        if (is_room_encryption(e)) {
                                mark_encrypted_room(RoomId(room_id));
                                console->debug("{}", get_json(e));
                        }
                }

                for (const auto &e : room.second.timeline.events) {
                        if (is_room_encryption(e)) {
                                mark_encrypted_room(RoomId(room_id));
                                console->debug("{}", get_json(e));
                        } else if (is_encrypted(e)) {
                                console->info("received an encrypted event: {}", room_id);
                                console->info("{}", get_json(e));

                                auto msg = mpark::get<EncryptedEvent<msg::Encrypted>>(e);

                                if (storage.inbound_group_exists(
                                      room_id, msg.content.session_id, msg.content.sender_key)) {
                                        auto res = olm_client->decrypt_group_message(
                                          storage.get_inbound_group_session(room_id,
                                                                            msg.content.session_id,
                                                                            msg.content.sender_key),
                                          msg.content.ciphertext);

                                        auto msg_str =
                                          std::string((char *)res.data.data(), res.data.size());

                                        console->info("decrypted data: {}", msg_str);
                                        console->info("decrypted message_index: {}",
                                                      res.message_index);
                                } else {
                                        console->warn(
                                          "no megolm session found to decrypt the event");
                                }
                        }
                }
        }
}

// Callback to executed after a /sync request completes.
void
sync_handler(const mtx::responses::Sync &res, RequestErr err)
{
        SyncOpts opts;

        if (err) {
                console->error("error during sync");
                print_errors(err);
                opts.since = client->next_batch_token();
                client->sync(opts, &sync_handler);
                return;
        }

        parse_messages(res);

        opts.since = res.next_batch;
        client->set_next_batch_token(res.next_batch);
        client->sync(opts, &sync_handler);
}

// Callback to executed after the first (initial) /sync request completes.
void
initial_sync_handler(const mtx::responses::Sync &res, RequestErr err)
{
        SyncOpts opts;

        if (err) {
                console->error("error during initial sync");
                print_errors(err);

                if (err->status_code != boost::beast::http::status::ok) {
                        console->error("retrying initial sync ..");
                        opts.timeout = 0;
                        client->sync(opts, &initial_sync_handler);
                }

                return;
        }

        parse_messages(res);

        for (const auto &room : res.rooms.join) {
                for (const auto &e : room.second.state.events) {
                        if (is_member_event(e)) {
                                auto m =
                                  mpark::get<mtx::events::StateEvent<mtx::events::state::Member>>(
                                    e);

                                get_device_keys(UserId(m.state_key));
                        }
                }
        }

        opts.since = res.next_batch;
        client->set_next_batch_token(res.next_batch);
        client->sync(opts, &sync_handler);
}

void
save_device_keys(const mtx::responses::QueryKeys &res)
{
        for (const auto &entry : res.device_keys) {
                const auto user_id = entry.first;

                if (!exists(storage.devices_, user_id))
                        console->info("keys for {}", user_id);

                std::vector<std::string> device_list;
                for (const auto &device : entry.second) {
                        const auto key_struct = device.second;

                        const std::string device_id = key_struct.device_id;
                        const std::string index     = "curve25519:" + device_id;

                        if (key_struct.keys.find(index) == key_struct.keys.end())
                                continue;

                        const auto key = key_struct.keys.at(index);

                        if (!exists(storage.device_keys_, device_id)) {
                                console->info("{} => {}", device_id, key);
                                storage.device_keys_[device_id] = key;
                        }

                        device_list.push_back(device_id);
                }

                if (!exists(storage.devices_, user_id)) {
                        storage.devices_[user_id] = device_list;
                }
        }
}

void
get_device_keys(const UserId &user)
{
        // Retrieve all devices keys.
        mtx::requests::QueryKeys query;
        query.device_keys[user.get()] = {};

        client->query_keys(query, [](const mtx::responses::QueryKeys &res, RequestErr err) {
                if (err) {
                        print_errors(err);
                        return;
                }

                for (const auto &key : res.device_keys) {
                        const auto user_id = key.first;
                        const auto devices = key.second;

                        for (const auto &device : devices) {
                                const auto id          = device.first;
                                const auto data        = device.second;
                                const auto signing_key = data.keys.at("ed25519:" + id);

                                try {
                                        auto ok = verify_identity_signature(
                                          json(data), DeviceId(id), UserId(user_id), signing_key);

                                        if (!ok) {
                                                console->warn("signature could not be verified");
                                                console->warn(json(data).dump(2));
                                        }
                                } catch (const olm_exception &e) {
                                        console->warn(e.what());
                                }
                        }
                }

                save_device_keys(std::move(res));
        });
}

void
handle_to_device_msgs(const std::vector<nlohmann::json> &msgs)
{
        if (!msgs.empty())
                console->info("inspecting {} to_device messages", msgs.size());

        for (const auto &msg : msgs) {
                console->info(msg.dump(2));

                try {
                        OlmMessage olm_msg = msg;
                        decrypt_olm_message(std::move(olm_msg));
                } catch (const nlohmann::json::exception &e) {
                        console->warn("parsing error for olm message: {}", e.what());
                } catch (const std::invalid_argument &e) {
                        console->warn("validation error for olm message: {}", e.what());
                }
        }
}

void
login_cb(const mtx::responses::Login &, RequestErr err)
{
        if (err) {
                console->error("login error");
                print_errors(err);
                return;
        }

        console->info("User ID: {}", client->user_id().to_string());
        console->info("Device ID: {}", client->device_id());

        // Upload one time keys.
        olm_client->set_user_id(client->user_id().to_string());
        olm_client->set_device_id(client->device_id());
        olm_client->generate_one_time_keys(50);

        client->upload_keys(olm_client->create_upload_keys_request(),
                            [](const mtx::responses::UploadKeys &, RequestErr err) {
                                    if (err) {
                                            print_errors(err);
                                            return;
                                    }

                                    console->info("keys uploaded");
                                    console->debug("starting initial sync");

                                    SyncOpts opts;
                                    opts.timeout = 0;
                                    client->sync(opts, &initial_sync_handler);
                            });
}

void
join_room_cb(const nlohmann::json &obj, RequestErr err)
{
        if (err) {
                print_errors(err);
                return;
        }

        (void)obj;

        // Fetch device list for all users.
}

int
main()
{
        spdlog::set_pattern("[%H:%M:%S] [tid %t] [%^%l%$] %v");

        std::string username;
        std::string server;
        std::string password;

        cout << "username: ";
        std::getline(std::cin, username);

        cout << "server: ";
        std::getline(std::cin, server);

        password = getpass("password: ");

        client = std::make_shared<Client>(server);

        olm_client = make_shared<OlmClient>();
        olm_client->create_new_account();

        console->info("ed25519: {}", olm_client->identity_keys().ed25519);
        console->info("curve25519: {}", olm_client->identity_keys().curve25519);

        client->login(username, password, login_cb);
        client->close();

        return 0;
}
