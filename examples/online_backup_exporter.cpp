#include <fstream>
#include <iostream>
#include <variant>

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

namespace {
std::shared_ptr<Client> client = nullptr;
}

void
print_errors(RequestErr err)
{
        if (!err->parse_error.empty())
                cerr << err->parse_error;
        if (!err->matrix_error.error.empty())
                cerr << err->matrix_error.error;
        if (err->error_code)
                cerr << err->error_code.message();
        if (err->status_code != boost::beast::http::status::unknown)
                cerr << err->status_code;
        cerr << "\n";
}

void
login_handler(const mtx::responses::Login &res, RequestErr err)
{
        if (err) {
                cerr << "There was an error during login: " << err->matrix_error.error << "\n";
                return;
        }

        cout << "Logged in as: " << res.user_id.to_string() << "\n";

        SyncOpts opts;
        opts.timeout = 0;

        client->set_access_token(res.access_token);

        client->backup_version(
          [](const mtx::responses::backup::BackupVersion &backup_version, RequestErr err) {
                  if (err) {
                          cerr << "Error fetching the backup version: ";
                          print_errors(err);
                          client->logout([](mtx::responses::Logout, RequestErr) {});
                          return;
                  }

                  if (backup_version.algorithm != mtx::responses::backup::megolm_backup_v1) {
                          cerr << "Incompatible backup algorithm: " << backup_version.algorithm
                               << "\n";
                          client->logout([](mtx::responses::Logout, RequestErr) {});
                          return;
                  }
                  client->room_keys(
                    backup_version.version,
                    [](mtx::responses::backup::KeysBackup backup, RequestErr err) {
                            if (err) {
                                    cerr << "Error fetching the backup: ";
                                    print_errors(err);
                                    return;
                                    client->logout([](mtx::responses::Logout, RequestErr) {});
                            }

                            cout << nlohmann::json(backup).dump(4) << "\n";
                            client->logout([](mtx::responses::Logout, RequestErr) {});
                    });
          });
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
