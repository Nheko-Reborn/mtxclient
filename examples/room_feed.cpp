#include <iostream>
#include <variant>

#include <unistd.h>

#include "mtx.hpp"
#include "mtxclient/http/client.hpp"
#include "mtxclient/http/errors.hpp"

//
// Simple usage example of the /login & /sync endpoints which
// will print the stream of messages from all rooms as received by the client.
//

using namespace std;
using namespace mtx::client;
using namespace mtx::http;
using namespace mtx::events;

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

// Simple print of the message contents.
void
print_message(const TimelineEvent &event)
{
    if (is_room_message(event))
        cout << get_sender(event) << ": " << get_body(event) << "\n";
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

    for (const auto &room : res.rooms.join) {
        for (const auto &msg : room.second.timeline.events)
            print_message(msg);
    }

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

    opts.since = res.next_batch;
    client->set_next_batch_token(res.next_batch);
    client->sync(opts, &sync_handler);
}

void
login_handler(const mtx::responses::Login &res, RequestErr err)
{
    if (err) {
        cout << "There was an error during login: " << err->matrix_error.error << "\n";
        return;
    }

    cout << "Logged in as: " << res.user_id.to_string() << "\n";
    SyncOpts opts;
    opts.timeout = 0;

    client->set_access_token(res.access_token);
    client->sync(opts, &initial_sync_handler);
}

int
main()
{
    std::string username, server, password;

    cout << "Username: ";
    std::getline(std::cin, username);

    cout << "HomeServer: ";
    std::getline(std::cin, server);

    password = getpass("Password: ");

    client = std::make_shared<Client>(server);
    client->login(username, password, &login_handler);
    client->close();

    return 0;
}
