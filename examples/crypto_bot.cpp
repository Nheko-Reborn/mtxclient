#include <csignal>
#include <cstdlib>

#include "spdlog/sinks/stdout_color_sinks.h"
#include "spdlog/spdlog.h"

#include <atomic>
#include <cstdint>
#include <fstream>
#include <iostream>
#include <nlohmann/json.hpp>
#include <stdexcept>
#include <thread>
#include <variant>

#include <mtx.hpp>
#include <mtx/identifiers.hpp>

#include "mtxclient/crypto/client.hpp"
#include "mtxclient/http/client.hpp"
#include "mtxclient/http/errors.hpp"

#include "mtxclient/utils.hpp"

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

constexpr auto OLM_ALGO    = "m.olm.v1.curve25519-aes-sha2";
constexpr auto STORAGE_KEY = "secret";

struct OlmCipherContent
{
    std::string body;
    uint8_t type;
};

inline void
from_json(const nlohmann::json &obj, OlmCipherContent &msg)
{
    msg.body = obj.at("body").get<std::string>();
    msg.type = obj.at("type").get<uint8_t>();
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

    msg.sender     = obj.at("sender").get<std::string>();
    msg.sender_key = obj.at("content").at("sender_key").get<std::string>();
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
handle_to_device_msgs(const mtx::responses::ToDevice &to_device);

struct OutboundSessionData
{
    std::string session_id;
    std::string session_key;
    uint64_t message_index = 0;
};

inline void
to_json(nlohmann::json &obj, const OutboundSessionData &msg)
{
    obj["session_id"]    = msg.session_id;
    obj["session_key"]   = msg.session_key;
    obj["message_index"] = msg.message_index;
}

inline void
from_json(const nlohmann::json &obj, OutboundSessionData &msg)
{
    msg.session_id    = obj.at("session_id").get<std::string>();
    msg.session_key   = obj.at("session_key").get<std::string>();
    msg.message_index = obj.at("message_index").get<uint64_t>();
}

struct OutboundSessionDataRef
{
    OlmOutboundGroupSession *session;
    OutboundSessionData data;
};

struct DevKeys
{
    std::string ed25519;
    std::string curve25519;
};

inline void
to_json(nlohmann::json &obj, const DevKeys &msg)
{
    obj["ed25519"]    = msg.ed25519;
    obj["curve25519"] = msg.curve25519;
}

inline void
from_json(const nlohmann::json &obj, DevKeys &msg)
{
    msg.ed25519    = obj.at("ed25519").get<std::string>();
    msg.curve25519 = obj.at("curve25519").get<std::string>();
}

auto console = spdlog::stdout_color_mt("console");

std::shared_ptr<Client> client        = nullptr;
std::shared_ptr<OlmClient> olm_client = nullptr;

struct Storage
{
    //! Storage for the user_id -> list of devices mapping.
    std::map<std::string, std::vector<std::string>> devices;
    //! Storage for the identity key for a device.
    std::map<std::string, DevKeys> device_keys;
    //! Flag that indicate if a specific room has encryption enabled.
    std::map<std::string, bool> encrypted_rooms;

    //! Keep track of members per room.
    std::map<std::string, std::map<std::string, bool>> members;

    void add_member(const std::string &room_id, const std::string &user_id)
    {
        members[room_id][user_id] = true;
    }

    //! Mapping from curve25519 to session.
    std::map<std::string, OlmSessionPtr> olm_inbound_sessions;
    std::map<std::string, OlmSessionPtr> olm_outbound_sessions;

    // TODO: store message_index / event_id
    std::map<std::string, InboundGroupSessionPtr> inbound_group_sessions;
    // TODO: store rotation period
    std::map<std::string, OutboundSessionData> outbound_group_session_data;
    std::map<std::string, OutboundGroupSessionPtr> outbound_group_sessions;

    bool outbound_group_exists(const std::string &room_id)
    {
        return (outbound_group_sessions.find(room_id) != outbound_group_sessions.end()) &&
               (outbound_group_session_data.find(room_id) != outbound_group_session_data.end());
    }

    void set_outbound_group_session(const std::string &room_id,
                                    OutboundGroupSessionPtr session,
                                    OutboundSessionData data)
    {
        outbound_group_session_data[room_id] = data;
        outbound_group_sessions[room_id]     = std::move(session);
    }

    OutboundSessionDataRef get_outbound_group_session(const std::string &room_id)
    {
        return OutboundSessionDataRef{outbound_group_sessions[room_id].get(),
                                      outbound_group_session_data[room_id]};
    }

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

    void load()
    {
        console->info("restoring storage");

        ifstream db("db.json");
        string db_data((istreambuf_iterator<char>(db)), istreambuf_iterator<char>());

        if (db_data.empty())
            return;

        nlohmann::json obj = nlohmann::json::parse(db_data);

        devices         = obj.at("devices").get<map<string, vector<string>>>();
        device_keys     = obj.at("device_keys").get<map<string, DevKeys>>();
        encrypted_rooms = obj.at("encrypted_rooms").get<map<string, bool>>();
        members         = obj.at("members").get<map<string, map<string, bool>>>();

        if (obj.count("olm_inbound_sessions") != 0) {
            auto sessions = obj.at("olm_inbound_sessions").get<map<string, string>>();
            for (const auto &s : sessions)
                olm_inbound_sessions[s.first] = unpickle<SessionObject>(s.second, STORAGE_KEY);
        }

        if (obj.count("olm_outbound_sessions") != 0) {
            auto sessions = obj.at("olm_outbound_sessions").get<map<string, string>>();
            for (const auto &s : sessions)
                olm_outbound_sessions[s.first] = unpickle<SessionObject>(s.second, STORAGE_KEY);
        }

        if (obj.count("inbound_group_sessions") != 0) {
            auto sessions = obj.at("inbound_group_sessions").get<map<string, string>>();
            for (const auto &s : sessions)
                inbound_group_sessions[s.first] =
                  unpickle<InboundSessionObject>(s.second, STORAGE_KEY);
        }

        if (obj.count("outbound_group_sessions") != 0) {
            auto sessions = obj.at("outbound_group_sessions").get<map<string, string>>();
            for (const auto &s : sessions)
                outbound_group_sessions[s.first] =
                  unpickle<OutboundSessionObject>(s.second, STORAGE_KEY);
        }

        if (obj.count("outbound_group_session_data") != 0) {
            auto sessions =
              obj.at("outbound_group_session_data").get<map<string, OutboundSessionData>>();
            for (const auto &s : sessions)
                outbound_group_session_data[s.first] = s.second;
        }
    }

    void save()
    {
        console->info("saving storage");

        std::ofstream db("db.json");
        if (!db.is_open()) {
            console->error("couldn't open file to save keys");
            return;
        }

        nlohmann::json data;
        data["devices"]         = devices;
        data["device_keys"]     = device_keys;
        data["encrypted_rooms"] = encrypted_rooms;
        data["members"]         = members;

        // Save inbound sessions
        for (const auto &s : olm_inbound_sessions)
            data["olm_inbound_sessions"][s.first] =
              mtx::crypto::pickle<SessionObject>(s.second.get(), STORAGE_KEY);

        for (const auto &s : olm_outbound_sessions)
            data["olm_outbound_sessions"][s.first] =
              mtx::crypto::pickle<SessionObject>(s.second.get(), STORAGE_KEY);

        for (const auto &s : inbound_group_sessions)
            data["inbound_group_sessions"][s.first] =
              mtx::crypto::pickle<InboundSessionObject>(s.second.get(), STORAGE_KEY);

        for (const auto &s : outbound_group_sessions)
            data["outbound_group_sessions"][s.first] =
              mtx::crypto::pickle<OutboundSessionObject>(s.second.get(), STORAGE_KEY);

        for (const auto &s : outbound_group_session_data)
            data["outbound_group_session_data"][s.first] = s.second;

        // Save to file
        db << data.dump(2);
        db.close();
    }
};

namespace {
Storage storage;
}

void
print_errors(RequestErr err)
{
    if (err->status_code)
        console->error("status code: {}", static_cast<uint16_t>(err->status_code));
    if (!err->matrix_error.error.empty())
        console->error("matrix error: {}", err->matrix_error.error);
    if (err->error_code)
        console->error("error code: {}", err->error_code);
}

template<class T>
bool
is_room_encryption(const T &event)
{
    using namespace mtx::events;
    using namespace mtx::events::state;
    return std::holds_alternative<StateEvent<Encryption>>(event);
}

void
send_group_message(OlmOutboundGroupSession *session,
                   const std::string &session_id,
                   const std::string &room_id,
                   const std::string &msg)
{
    // Create event payload
    nlohmann::json doc{{"type", "m.room.message"},
                       {"content", {{"type", "m.text"}, {"body", msg}}},
                       {"room_id", room_id}};

    auto payload = olm_client->encrypt_group_message(session, doc.dump());

    using namespace mtx::events;
    using namespace mtx::identifiers;

    msg::Encrypted data;
    data.ciphertext = std::string((char *)payload.data(), payload.size());
    data.sender_key = olm_client->identity_keys().curve25519;
    data.session_id = session_id;
    data.device_id  = client->device_id();

    client->send_room_message<msg::Encrypted>(
      room_id, data, [](const mtx::responses::EventId &res, RequestErr err) {
          if (err) {
              print_errors(err);
              return;
          }

          console->info("message sent with event_id: {}", res.event_id.to_string());
      });
}

void
create_outbound_megolm_session(const std::string &room_id, const std::string &reply_msg)
{
    // Create an outbound session
    auto outbound_session = olm_client->init_outbound_group_session();

    const auto session_id  = mtx::crypto::session_id(outbound_session.get());
    const auto session_key = mtx::crypto::session_key(outbound_session.get());

    mtx::events::DeviceEvent<mtx::events::msg::RoomKey> megolm_payload;
    megolm_payload.content.algorithm   = "m.megolm.v1.aes-sha2";
    megolm_payload.content.room_id     = room_id;
    megolm_payload.content.session_id  = session_id;
    megolm_payload.content.session_key = session_key;
    megolm_payload.type                = mtx::events::EventType::RoomKey;

    if (storage.members.find(room_id) == storage.members.end()) {
        console->error("no members found for room {}", room_id);
        return;
    }

    const auto members = storage.members[room_id];

    for (const auto &member : members) {
        const auto devices = storage.devices[member.first];

        // TODO: Figure out for which devices we don't have olm sessions.
        for (const auto &dev : devices) {
            // TODO: check if we have downloaded the keys
            const auto device_keys = storage.device_keys[dev];

            auto to_device_cb = [](RequestErr err) {
                if (err) {
                    print_errors(err);
                }
            };

            if (storage.olm_outbound_sessions.find(device_keys.curve25519) !=
                storage.olm_outbound_sessions.end()) {
                console->info("found existing olm outbound session with device {}", dev);
                auto olm_session = storage.olm_outbound_sessions[device_keys.curve25519].get();

                auto device_msg = olm_client->create_olm_encrypted_content(olm_session,
                                                                           megolm_payload,
                                                                           UserId(member.first),
                                                                           device_keys.ed25519,
                                                                           device_keys.curve25519);

                nlohmann::json body{{"messages", {{member, {{dev, device_msg}}}}}};

                client->send_to_device("m.room.encrypted", body, to_device_cb);
                // TODO: send message to device
            } else {
                console->info("claiming one time keys for device {}", dev);
                auto cb = [member = member.first, dev, megolm_payload, to_device_cb](
                            const mtx::responses::ClaimKeys &res, RequestErr err) {
                    if (err) {
                        print_errors(err);
                        return;
                    }

                    console->info("claimed keys for {} - {}", member, dev);
                    console->info("room_key {}", nlohmann::json(megolm_payload).dump(4));

                    console->warn("signed one time keys");
                    auto retrieved_devices = res.one_time_keys.at(member);
                    for (const auto &rd : retrieved_devices) {
                        console->info("{} : \n {}", rd.first, rd.second.dump(2));

                        // TODO: Verify signatures
                        auto otk    = rd.second.begin()->at("key");
                        auto id_key = storage.device_keys[dev].curve25519;

                        auto session =
                          olm_client->create_outbound_session(id_key, otk.get<std::string>());

                        auto device_msg = olm_client->create_olm_encrypted_content(
                          session.get(),
                          megolm_payload,
                          UserId(member),
                          storage.device_keys[dev].ed25519,
                          storage.device_keys[dev].curve25519);

                        // TODO: saving should happen when the message is
                        // sent.
                        storage.olm_outbound_sessions[id_key] = std::move(session);

                        nlohmann::json body{{"messages", {{member, {{dev, device_msg}}}}}};

                        client->send_to_device("m.room.encrypted", body, to_device_cb);
                    }
                };

                mtx::requests::ClaimKeys claim_keys;
                claim_keys.one_time_keys[member.first][dev] = SIGNED_CURVE25519;

                // TODO: we should bulk request device keys here
                client->claim_keys(claim_keys, cb);
            }
        }
    }

    console->info("waiting to send sendToDevice messages");
    std::this_thread::sleep_for(std::chrono::milliseconds(2000));
    console->info("sending encrypted group message");

    // TODO: This should be done after all sendToDevice messages have been sent.
    send_group_message(outbound_session.get(), session_id, room_id, reply_msg);

    // TODO: save message index also.
    storage.set_outbound_group_session(
      room_id, std::move(outbound_session), {session_id, session_key});
}

bool
is_encrypted(const TimelineEvent &event)
{
    using namespace mtx::events;
    return std::holds_alternative<EncryptedEvent<msg::Encrypted>>(event);
}

template<class T>
bool
is_member_event(const T &event)
{
    using namespace mtx::events;
    using namespace mtx::events::state;
    return std::holds_alternative<StateEvent<Member>>(event);
}

// Check if the given event has a textual representation.
bool
is_room_message(const TimelineEvent &e)
{
    return (std::holds_alternative<mtx::events::RoomEvent<msg::Audio>>(e)) ||
           (std::holds_alternative<mtx::events::RoomEvent<msg::Emote>>(e)) ||
           (std::holds_alternative<mtx::events::RoomEvent<msg::File>>(e)) ||
           (std::holds_alternative<mtx::events::RoomEvent<msg::Image>>(e)) ||
           (std::holds_alternative<mtx::events::RoomEvent<msg::Notice>>(e)) ||
           (std::holds_alternative<mtx::events::RoomEvent<msg::Text>>(e)) ||
           (std::holds_alternative<mtx::events::RoomEvent<msg::Video>>(e));
}

// Retrieves the fallback body value from the event.
std::string
get_body(const TimelineEvent &e)
{
    if (auto ev = std::get_if<RoomEvent<msg::Audio>>(&e); ev != nullptr)
        return ev->content.body;
    else if (auto ev = std::get_if<RoomEvent<msg::Emote>>(&e); ev != nullptr)
        return ev->content.body;
    else if (auto ev = std::get_if<RoomEvent<msg::File>>(&e); ev != nullptr)
        return ev->content.body;
    else if (auto ev = std::get_if<RoomEvent<msg::Image>>(&e); ev != nullptr)
        return ev->content.body;
    else if (auto ev = std::get_if<RoomEvent<msg::Notice>>(&e); ev != nullptr)
        return ev->content.body;
    else if (auto ev = std::get_if<RoomEvent<msg::Text>>(&e); ev != nullptr)
        return ev->content.body;
    else if (auto ev = std::get_if<RoomEvent<msg::Video>>(&e); ev != nullptr)
        return ev->content.body;

    return "";
}

// Retrieves the sender of the event.
std::string
get_sender(const TimelineEvent &event)
{
    return std::visit([](auto e) { return e.sender; }, event);
}

template<class T>
std::string
get_json(const T &event)
{
    return std::visit([](auto e) { return nlohmann::json(e).dump(2); }, event);
}

void
keys_uploaded_cb(const mtx::responses::UploadKeys &, RequestErr err)
{
    if (err) {
        print_errors(err);
        return;
    }

    olm_client->mark_keys_as_published();
    console->info("keys uploaded");
}

void
mark_encrypted_room(const RoomId &id)
{
    console->info("encryption is enabled for room: {}", id.get());
    storage.encrypted_rooms[id.get()] = true;
}

void
send_encrypted_reply(const std::string &room_id, const std::string &reply_msg)
{
    console->info("sending reply");

    // Create a megolm session if it doesn't already exist.
    if (storage.outbound_group_exists(room_id)) {
        auto session_obj = storage.get_outbound_group_session(room_id);

        send_group_message(session_obj.session, session_obj.data.session_id, room_id, reply_msg);

    } else {
        console->info("creating new megolm outbound session");
        create_outbound_megolm_session(room_id, reply_msg);
    }
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

                auto ok =
                  matches_inbound_session_from(inbound_session.get(), olm_msg.sender_key, msg_body);

                if (!ok) {
                    console->error("session could not be established");

                } else {
                    auto output =
                      olm_client->decrypt_message(inbound_session.get(), msg_type, msg_body);

                    auto plaintext =
                      nlohmann::json::parse(std::string((char *)output.data(), output.size()));
                    console->info("decrypted message: \n {}", plaintext.dump(2));

                    storage.olm_inbound_sessions.emplace(olm_msg.sender_key,
                                                         std::move(inbound_session));

                    std::string room_id = plaintext.at("content").at("room_id").get<std::string>();
                    std::string session_id =
                      plaintext.at("content").at("session_id").get<std::string>();
                    std::string session_key =
                      plaintext.at("content").at("session_key").get<std::string>();

                    if (storage.inbound_group_exists(room_id, session_id, olm_msg.sender_key)) {
                        console->warn("megolm session already exists");
                    } else {
                        auto megolm_session = olm_client->init_inbound_group_session(session_key);

                        storage.set_inbound_group_session(
                          room_id, session_id, olm_msg.sender_key, std::move(megolm_session));

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
        auto room_id = room.first;

        console->info("joining room {}", room_id);
        client->join_room(room_id, [room_id](const mtx::responses::RoomId &, RequestErr e) {
            if (e) {
                print_errors(e);
                console->error("failed to join room {}", room_id);
                return;
            }
        });
    }

    // Check if we have any new m.room_key messages (i.e starting a new megolm session)
    handle_to_device_msgs(res.to_device);

    // Check if the uploaded one time keys are enough
    for (const auto &device : res.device_one_time_keys_count) {
        if (device.second < 50) {
            console->info("number of one time keys: {}", device.second);
            olm_client->generate_one_time_keys(50 - device.second);
            // TODO: Mark keys as sent
            client->upload_keys(olm_client->create_upload_keys_request(), &keys_uploaded_cb);
        }
    }

    for (const auto &room : res.rooms.join) {
        const std::string room_id = room.first;

        for (const auto &e : room.second.state.events) {
            if (is_room_encryption(e)) {
                mark_encrypted_room(RoomId(room_id));
                console->debug("{}", get_json(e));
            } else if (is_member_event(e)) {
                auto m = std::get<mtx::events::StateEvent<mtx::events::state::Member>>(e);

                get_device_keys(UserId(m.state_key));
                storage.add_member(room_id, m.state_key);
            }
        }

        for (const auto &e : room.second.timeline.events) {
            if (is_room_encryption(e)) {
                mark_encrypted_room(RoomId(room_id));
                console->debug("{}", get_json(e));
            } else if (is_member_event(e)) {
                auto m = std::get<mtx::events::StateEvent<mtx::events::state::Member>>(e);

                get_device_keys(UserId(m.state_key));
                storage.add_member(room_id, m.state_key);
            } else if (is_encrypted(e)) {
                console->info("received an encrypted event: {}", room_id);
                console->info("{}", get_json(e));

                auto msg = std::get<EncryptedEvent<msg::Encrypted>>(e);

                if (storage.inbound_group_exists(
                      room_id, msg.content.session_id, msg.content.sender_key)) {
                    auto res = olm_client->decrypt_group_message(
                      storage.get_inbound_group_session(
                        room_id, msg.content.session_id, msg.content.sender_key),
                      msg.content.ciphertext);

                    auto msg_str = std::string((char *)res.data.data(), res.data.size());
                    const auto body =
                      nlohmann::json::parse(msg_str).at("content").at("body").get<std::string>();

                    console->info("decrypted data: {}", body);
                    console->info("decrypted message_index: {}", res.message_index);

                    if (msg.sender != client->user_id().to_string()) {
                        // Send a reply back to the sender.
                        std::string reply_txt(msg.sender + ": you said '" + body + "'");
                        send_encrypted_reply(room_id, reply_txt);
                    }

                } else {
                    console->warn("no megolm session found to decrypt the event");
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

        if (err->status_code != 200) {
            console->error("retrying initial sync ..");
            opts.timeout = 0;
            client->sync(opts, &initial_sync_handler);
        }

        return;
    }

    parse_messages(res);

    for (const auto &room : res.rooms.join) {
        const auto room_id = room.first;

        for (const auto &e : room.second.state.events) {
            if (is_member_event(e)) {
                auto m = std::get<mtx::events::StateEvent<mtx::events::state::Member>>(e);

                get_device_keys(UserId(m.state_key));
                storage.add_member(room_id, m.state_key);
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

        if (!exists(storage.devices, user_id))
            console->info("keys for {}", user_id);

        std::vector<std::string> device_list;
        for (const auto &device : entry.second) {
            const auto key_struct = device.second;

            const std::string device_id = key_struct.device_id;
            const std::string index     = "curve25519:" + device_id;

            if (key_struct.keys.find(index) == key_struct.keys.end())
                continue;

            const auto key = key_struct.keys.at(index);

            if (!exists(storage.device_keys, device_id)) {
                console->info("{} => {}", device_id, key);
                storage.device_keys[device_id] = {key_struct.keys.at("ed25519:" + device_id),
                                                  key_struct.keys.at("curve25519:" + device_id)};
            }

            device_list.push_back(device_id);
        }

        if (!exists(storage.devices, user_id)) {
            storage.devices[user_id] = device_list;
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
                const auto id   = device.first;
                const auto data = device.second;

                try {
                    auto ok =
                      verify_identity_signature(nlohmann::json(data).get<mtx::crypto::DeviceKeys>(),
                                                DeviceId(id),
                                                UserId(user_id));

                    if (!ok) {
                        console->warn("signature could not be verified");
                        console->warn(nlohmann::json(data).dump(2));
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
handle_to_device_msgs(const mtx::responses::ToDevice &msgs)
{
    if (!msgs.events.empty())
        console->info("inspecting {} to_device messages", msgs.events.size());

    for (const auto &msg : msgs.events) {
        console->info(std::visit([](const auto &e) { return nlohmann::json(e); }, msg).dump(2));

        try {
            OlmMessage olm_msg =
              std::visit([](const auto &e) { return nlohmann::json(e).get<OlmMessage>(); }, msg);
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
    console->info("ed25519: {}", olm_client->identity_keys().ed25519);
    console->info("curve25519: {}", olm_client->identity_keys().curve25519);

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

                            olm_client->mark_keys_as_published();
                            console->info("keys uploaded");
                            console->debug("starting initial sync");

                            SyncOpts opts;
                            opts.timeout = 0;
                            client->sync(opts, &initial_sync_handler);
                        });
}

void
join_room_cb(const mtx::responses::RoomId &, RequestErr err)
{
    if (err) {
        print_errors(err);
        return;
    }

    // Fetch device list for all users.
}

void
shutdown_handler(int sig)
{
    console->warn("received {} signal", sig);
    storage.save();

    std::ofstream db("account.json");
    if (!db.is_open()) {
        console->error("couldn't open file to save account keys");
        return;
    }

    nlohmann::json data;
    data["account"] = olm_client->save(STORAGE_KEY);

    db << data.dump(2);
    db.close();

    // The sync calls will stop.
    client->shutdown();
}

int
main()
{
    spdlog::set_pattern("[%H:%M:%S] [tid %t] [%^%l%$] %v");

    std::signal(SIGINT, shutdown_handler);

    std::string username("alice");
    std::string server("localhost");
    std::string password("secret");

    client = std::make_shared<Client>(server);

    olm_client = make_shared<OlmClient>();

    ifstream db("account.json");
    string db_data((istreambuf_iterator<char>(db)), istreambuf_iterator<char>());

    if (db_data.empty())
        olm_client->create_new_account();
    else
        olm_client->load(nlohmann::json::parse(db_data).at("account").get<std::string>(),
                         STORAGE_KEY);

    storage.load();

    client->login(username, password, login_cb);
    client->close();

    console->info("exit");

    return 0;
}
