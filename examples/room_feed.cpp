#include <boost/beast.hpp>
#include <boost/variant.hpp>
#include <iostream>
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

        for (const auto room : res.rooms.join) {
                for (const auto msg : room.second.timeline.events)
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

                if (err->status_code != boost::beast::http::status::ok) {
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
