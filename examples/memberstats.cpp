#include <fstream>
#include <iostream>

#include <unistd.h>

#include <nlohmann/json.hpp>

#include "mtx.hpp"
#include "mtxclient/http/client.hpp"
#include "mtxclient/http/errors.hpp"

//
// Simple usage example using the members and state endpoints to iterate all members to their first
// join. Will write to history.tsv the timestamp and userid
//

using namespace std;
using namespace mtx::client;
using namespace mtx::http;
using namespace mtx::events;

namespace {
std::shared_ptr<Client> client = nullptr;
}

void
print_errors(RequestErr err)
{
    if (err->status_code)
        cerr << err->status_code << "\n";
    if (!err->matrix_error.error.empty())
        cerr << err->matrix_error.error << "\n";
    if (err->error_code)
        cerr << err->error_code << "\n";
}

using Memberlist = decltype(mtx::responses::Members::chunk);

using Memberevent = decltype(mtx::responses::Members::chunk)::value_type;

struct HistoryEvent
{
    std::string user;
    uint64_t ts = 0;
    mtx::events::state::Membership membership;
};

std::vector<HistoryEvent> member_to_ts;
std::string roomid;

Memberlist members;

void
member_fetch_loop(Memberevent current)
{
    while (current.unsigned_data.replaces_state.empty()) {
        if (member_to_ts.empty() || member_to_ts.back().user != current.state_key ||
            member_to_ts.back().membership != current.content.membership) {
            if (current.content.membership == mtx::events::state::Membership::Join) {
                member_to_ts.push_back(
                  {current.state_key, current.origin_server_ts, current.content.membership});
                std::cerr << ",";
            } else if (current.content.membership == mtx::events::state::Membership::Leave) {
                member_to_ts.push_back(
                  {current.state_key, current.origin_server_ts, current.content.membership});
                std::cerr << ";";
            }
        }

        if (members.empty())
            return;

        current = members.back();
        members.pop_back();
    }

    if (member_to_ts.empty() || member_to_ts.back().user != current.state_key ||
        member_to_ts.back().membership != current.content.membership) {
        if (current.content.membership == mtx::events::state::Membership::Join) {
            member_to_ts.push_back(
              {current.state_key, current.origin_server_ts, current.content.membership});
            std::cerr << ".";
        } else if (current.content.membership == mtx::events::state::Membership::Leave) {
            member_to_ts.push_back(
              {current.state_key, current.origin_server_ts, current.content.membership});
            std::cerr << ":";
        }
    }

    client->get_event(
      roomid,
      current.unsigned_data.replaces_state,
      [](mtx::events::collections::TimelineEvents ev, RequestErr err) mutable {
          if (err) {
              cerr << "There was an error fetching a memberevent: ";
              print_errors(err);
          } else if (auto event = std::get_if<Memberevent>(&ev)) {
              member_fetch_loop(std::move(*event));
              return;
          }

          cerr << "Event wrong type, skipping: "
               << std::visit([](auto raw) { return nlohmann::json(raw).dump(2); }, ev) << "\n";
          std::cerr << "#";

          if (members.empty())
              return;

          auto first = members.back();
          members.pop_back();
          member_fetch_loop(std::move(first));
      });
}

void
login_handler(const mtx::responses::Login &res, RequestErr err)
{
    if (err) {
        cerr << "There was an error during login: " << err->matrix_error.error << "\n";
        return;
    }

    cerr << "Logged in as: " << res.user_id.to_string() << "\n";

    cerr << "give room id to download member history from: ";
    std::getline(std::cin, roomid);

    client->set_access_token(res.access_token);

    client->members(roomid, [](mtx::responses::Members members_, RequestErr err) {
        if (err) {
            std::cerr << "Failed to gather members: ";
            print_errors(err);
            return;
        }

        std::cerr << "Fetched members: " << members_.chunk.size();

        if (members_.chunk.empty())
            return;

        auto first = members_.chunk.back();
        members_.chunk.pop_back();
        members = std::move(members_.chunk);

        member_fetch_loop(std::move(first));
    });
}

int
main()
{
    std::string username, server, password;

    cerr << "Username: ";
    std::getline(std::cin, username);

    cerr << "HomeServer: ";
    std::getline(std::cin, server);

    password = getpass("Password: ");

    client = std::make_shared<Client>(server);
    client->login(username, password, &login_handler);
    client->close();

    std::ofstream file("history.tsv");
    file << "timestamp\tjoined\tuser\n";
    for (const auto &e : member_to_ts)
        file << e.ts << "\t" << (e.membership == mtx::events::state::Membership::Join ? "j" : "l")
             << "\t" << e.user << "\n";
    file.close();

    return 0;
}
