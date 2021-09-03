#include <unistd.h>

#include <iostream>
#include <variant>

#include <nlohmann/json.hpp>

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
    if (err->status_code)
        cout << err->status_code << "\n";
    if (!err->matrix_error.error.empty())
        cout << err->matrix_error.error << "\n";
    if (err->error_code)
        cout << err->error_code << "\n";
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

void
parse_messages(const mtx::responses::Sync &res, bool parse_repeat_cmd = false)
{
    for (const auto &room : res.rooms.invite) {
        auto room_id = room.first;

        printf("joining room %s\n", room_id.c_str());
        client->join_room(room_id, [room_id](const mtx::responses::RoomId &obj, RequestErr e) {
            if (e) {
                print_errors(e);
                printf("failed to join room %s\n", room_id.c_str());
                return;
            }

            printf("joined room \n%s\n", obj.room_id.c_str());

            mtx::events::msg::Text text;
            text.body = "Thanks for the invitation!";

            client->send_room_message<mtx::events::msg::Text>(
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

    for (const auto &room : res.rooms.join) {
        const std::string repeat_cmd = "!repeat";
        const std::string room_id    = room.first;

        for (const auto &e : room.second.timeline.events) {
            if (!is_room_message(e))
                continue;

            auto body = get_body(e);
            if (body.find(repeat_cmd) != 0)
                continue;

            auto word = std::string(body.begin() + repeat_cmd.size(), body.end());
            auto user = get_sender(e);

            mtx::events::msg::Text text;
            text.body = user + ": " + word;

            client->send_room_message<mtx::events::msg::Text>(
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

        if (err->status_code != 200) {
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
