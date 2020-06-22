#include <fstream>
#include <iostream>
#include <variant>

#include "mtx.hpp"
#include "mtxclient/crypto/utils.hpp"
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

        client->backup_version([](const mtx::responses::backup::BackupVersion &backup_version,
                                  RequestErr err) {
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
                                  client->logout([](mtx::responses::Logout, RequestErr) {});
                                  return;
                          }

                          cout << nlohmann::json(backup).dump(4) << "\n";

                          client->secret_storage_secret(
                            mtx::secret_storage::secrets::megolm_backup_v1,
                            [backup](mtx::secret_storage::Secret secret, RequestErr err) {
                                    if (err) {
                                            cerr << "Error fetching the backup secret: ";
                                            print_errors(err);
                                            client->logout(
                                              [](mtx::responses::Logout, RequestErr) {});
                                            return;
                                    }

                                    if (secret.encrypted.size() != 1) {
                                            cerr << "Only one encryption key for backup "
                                                    "supported. Aborting.\n";
                                            return;
                                            client->logout(
                                              [](mtx::responses::Logout, RequestErr) {});
                                            return;
                                    }

                                    client->secret_storage_key(
                                      secret.encrypted.begin()->first,
                                      [backup, secretData = secret.encrypted.begin()->second](
                                        mtx::secret_storage::AesHmacSha2KeyDescription keyDesc,
                                        RequestErr err) {
                                              client->logout(
                                                [](mtx::responses::Logout, RequestErr) {});
                                              if (err) {
                                                      cerr << "Error fetching the backup key "
                                                              "description: ";
                                                      print_errors(err);
                                                      return;
                                              }

                                              mtx::crypto::BinaryBuf decryptionKey;
                                              if (keyDesc.passphrase) {
                                                      auto password =
                                                        getpass("Enter Key Backup Password: ");
                                                      decryptionKey = mtx::crypto::derive_key(
                                                        password, keyDesc.passphrase.value());
                                              } else {
                                                      auto recoveryKey =
                                                        getpass("Enter Key Backup Recovery Key: ");
                                                      decryptionKey =
                                                        mtx::crypto::to_binary_buf(recoveryKey);
                                              }
                                              // BinaryBuf
                                              // AES_CTR_256_Encrypt(const std::string plaintext,
                                              // const BinaryBuf aes256Key, BinaryBuf iv);

                                              // verify key
                                              using namespace mtx::crypto;
                                              auto testKeys = HKDF_SHA256(
                                                decryptionKey, BinaryBuf(32, 0), BinaryBuf{});

                                              auto encrypted = AES_CTR_256_Encrypt(
                                                std::string(32, '\0'),
                                                testKeys.aes,
                                                to_binary_buf(base642bin(keyDesc.iv)));

                                              auto mac = HMAC_SHA256(testKeys.mac, encrypted);
                                              if (bin2base64(to_string(mac)) != keyDesc.mac) {
                                                      cerr
                                                        << "mac failed, key or password wrong!\n";
                                                      return;
                                              }

                                              auto keys   = HKDF_SHA256(decryptionKey,
                                                                      BinaryBuf(32, 0),
                                                                      to_binary_buf(keyDesc.name));
                                              auto keyMac = HMAC_SHA256(
                                                keys.mac,
                                                to_binary_buf(base642bin(secretData.ciphertext)));

                                              if (bin2base64(to_string(keyMac)) != secretData.mac) {
                                                      cerr
                                                        << bin2base64(to_string(keyMac)) << " and "
                                                        << secretData.mac
                                                        << " don't match, can't decrypt ecdh key!";
                                                      return;
                                              }

                                              // struct ExportedSession
                                              //{
                                              //        std::map<std::string, std::string>
                                              //        sender_claimed_keys;   // currently unused.
                                              //        std::vector<std::string>
                                              //        forwarding_curve25519_key_chain; //
                                              //        currently unused.
                                              //
                                              //        std::string algorithm = MEGOLM_ALGO;
                                              //        std::string room_id;
                                              //        std::string sender_key;
                                              //        std::string session_id;
                                              //        std::string session_key;
                                              //};
                                      });
                            });
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
