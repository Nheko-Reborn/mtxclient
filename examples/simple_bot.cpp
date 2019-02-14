#include <boost/algorithm/string/predicate.hpp>
#include <boost/beast.hpp>
#include <boost/variant.hpp>

#include <iostream>
#include <nlohmann/json.hpp>
#include <unistd.h>

#include <mtx.hpp>
#include <mtx/identifiers.hpp>

#include "mtxclient/http/client.hpp"
#include "mtxclient/http/errors.hpp"

//
// Simple example bot that will accept any invite.
//

using namespace std;
using namespace mtx::client;
using namespace mtx::http;
using namespace mtx::events;
using namespace mtx::identifiers;

using TimelineEvent = mtx::events::collections::TimelineEvents;

namespace {
std::shared_ptr<Client> client = nullptr;
}

void
print_errors(RequestErr err)
{
        if (err->status_code != boost::beast::http::status::unknown)
                cout << err->status_code << "\n";
        if (!err->matrix_error.error.empty())
                cout << err->matrix_error.error << "\n";
        if (err->error_code)
                cout << err->error_code.message() << "\n";
}

// Check if the given event has a textual representation.
bool
is_room_message(const TimelineEvent &e)
{
        return (boost::get<mtx::events::RoomEvent<msg::Audio>>(&e) != nullptr) ||
               (boost::get<mtx::events::RoomEvent<msg::Emote>>(&e) != nullptr) ||
               (boost::get<mtx::events::RoomEvent<msg::File>>(&e) != nullptr) ||
               (boost::get<mtx::events::RoomEvent<msg::Image>>(&e) != nullptr) ||
               (boost::get<mtx::events::RoomEvent<msg::Notice>>(&e) != nullptr) ||
               (boost::get<mtx::events::RoomEvent<msg::Text>>(&e) != nullptr) ||
               (boost::get<mtx::events::RoomEvent<msg::Video>>(&e) != nullptr);
}

// Retrieves the fallback body value from the event.
std::string
get_body(const TimelineEvent &e)
{
        if (boost::get<RoomEvent<msg::Audio>>(&e) != nullptr)
                return boost::get<RoomEvent<msg::Audio>>(e).content.body;
        else if (boost::get<RoomEvent<msg::Emote>>(&e) != nullptr)
                return boost::get<RoomEvent<msg::Emote>>(e).content.body;
        else if (boost::get<RoomEvent<msg::File>>(&e) != nullptr)
                return boost::get<RoomEvent<msg::File>>(e).content.body;
        else if (boost::get<RoomEvent<msg::Image>>(&e) != nullptr)
                return boost::get<RoomEvent<msg::Image>>(e).content.body;
        else if (boost::get<RoomEvent<msg::Notice>>(&e) != nullptr)
                return boost::get<RoomEvent<msg::Notice>>(e).content.body;
        else if (boost::get<RoomEvent<msg::Text>>(&e) != nullptr)
                return boost::get<RoomEvent<msg::Text>>(e).content.body;
        else if (boost::get<RoomEvent<msg::Video>>(&e) != nullptr)
                return boost::get<RoomEvent<msg::Video>>(e).content.body;

        return "";
}

// Retrieves the sender of the event.
std::string
get_sender(const TimelineEvent &event)
{
        return boost::apply_visitor([](auto e) { return e.sender; }, event);
}

void
parse_messages(const mtx::responses::Sync &res, bool parse_repeat_cmd = false)
{
        for (const auto room : res.rooms.invite) {
                auto room_id = room.first;

                printf("joining room %s\n", room_id.c_str());
                client->join_room(room_id, [room_id](const nlohmann::json &obj, RequestErr e) {
                        if (e) {
                                print_errors(e);
                                printf("failed to join room %s\n", room_id.c_str());
                                return;
                        }

                        printf("joined room \n%s\n", obj.dump(2).c_str());

                        mtx::events::msg::Text text;
                        text.body = "Thanks for the invitation!";

                        client->send_room_message<mtx::events::msg::Text,
                                                  mtx::events::EventType::RoomMessage>(
                          room_id, text, [room_id](const mtx::responses::EventId &, RequestErr e) {
                                  if (e) {
                                          print_errors(e);
                                          return;
                                  }

                                  printf("sent message to %s\n", room_id.c_str());
                          });
                });
        }

        if (!parse_repeat_cmd)
                return;

        for (const auto room : res.rooms.join) {
                const std::string repeat_cmd = "!repeat";
                const std::string room_id    = room.first;

                for (const auto &e : room.second.timeline.events) {
                        if (!is_room_message(e))
                                continue;

                        auto body = get_body(e);
                        if (!boost::starts_with(body, repeat_cmd))
                                continue;

                        auto word = std::string(body.begin() + repeat_cmd.size(), body.end());
                        auto user = get_sender(e);

                        mtx::events::msg::Text text;
                        text.body = user + ": " + word;

                        client->send_room_message<mtx::events::msg::Text,
                                                  mtx::events::EventType::RoomMessage>(
                          room_id, text, [room_id](const mtx::responses::EventId &, RequestErr e) {
                                  if (e) {
                                          print_errors(e);
                                          return;
                                  }

                                  printf("sent message to %s\n", room_id.c_str());
                          });
                }
        }
}

// Callback to executed after a /sync request completes.
void
sync_handler(const mtx::responses::Sync &res, RequestErr err)
{
        SyncOpts opts;

        if (err) {
                cout << "sync error:\n";
                print_errors(err);
                opts.since = client->next_batch_token();
                client->sync(opts, &sync_handler);
                return;
        }

        parse_messages(res, true);

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
                cout << "error during initial sync:\n";
                print_errors(err);

                if (err->status_code != boost::beast::http::status::ok) {
                        cout << "retrying initial sync ...\n";
                        opts.timeout = 0;
                        client->sync(opts, &initial_sync_handler);
                }

                return;
        }

        parse_messages(res);

        opts.since = res.next_batch;
        client->set_next_batch_token(res.next_batch);
        client->sync(opts, &sync_handler);
}

void
login_handler(const mtx::responses::Login &, RequestErr err)
{
        if (err) {
                printf("login error\n");
                print_errors(err);
                return;
        }

        printf("user_id: %s\n", client->user_id().to_string().c_str());
        printf("device_id: %s\n", client->device_id().c_str());

        SyncOpts opts;
        opts.timeout = 0;
        client->sync(opts, &initial_sync_handler);
}

int
main()
{
        std::string username, server, password;

        cout << "username: ";
        std::getline(std::cin, username);

        cout << "server: ";
        std::getline(std::cin, server);

        password = getpass("password: ");

        client = std::make_shared<Client>(server);
        client->login(username, password, login_handler);
        client->close();

        return 0;
}
