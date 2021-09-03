#include <fstream>
#include <iostream>
#include <variant>

#include <unistd.h>

#include "mtx.hpp"
#include "mtxclient/crypto/client.hpp"
#include "mtxclient/crypto/types.hpp"
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

static void
export_backup(const mtx::secret_storage::AesHmacSha2KeyDescription &keyDesc,
              const mtx::secret_storage::AesHmacSha2EncryptedData &secretData,
              const mtx::responses::backup::KeysBackup &backup);

void
print_errors(RequestErr err)
{
    if (!err->parse_error.empty())
        cerr << err->parse_error;
    if (!err->matrix_error.error.empty())
        cerr << err->matrix_error.error;
    if (err->error_code)
        cerr << err->error_code;
    if (err->status_code)
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
              cerr << "Incompatible backup algorithm: " << backup_version.algorithm << "\n";
              client->logout([](mtx::responses::Logout, RequestErr) {});
              return;
          }
          client->room_keys(
            backup_version.version, [](mtx::responses::backup::KeysBackup backup, RequestErr err) {
                if (err) {
                    cerr << "Error fetching the backup: ";
                    print_errors(err);
                    client->logout([](mtx::responses::Logout, RequestErr) {});
                    return;
                }

                client->secret_storage_secret(
                  mtx::secret_storage::secrets::megolm_backup_v1,
                  [backup](mtx::secret_storage::Secret secret, RequestErr err) {
                      if (err) {
                          cerr << "Error fetching the backup secret: ";
                          print_errors(err);
                          client->logout([](mtx::responses::Logout, RequestErr) {});
                          return;
                      }

                      if (secret.encrypted.size() != 1) {
                          cerr << "Only one encryption key for backup "
                                  "supported. Aborting.\n";
                          client->logout([](mtx::responses::Logout, RequestErr) {});
                          return;
                      }

                      client->secret_storage_key(
                        secret.encrypted.begin()->first,
                        [backup, secretData = secret.encrypted.begin()->second](
                          mtx::secret_storage::AesHmacSha2KeyDescription keyDesc, RequestErr err) {
                            client->logout([](mtx::responses::Logout, RequestErr) {});
                            if (err) {
                                cerr << "Error fetching the backup key "
                                        "description: ";
                                print_errors(err);
                                return;
                            }

                            export_backup(keyDesc, secretData, backup);
                        });
                  });
            });
      });
}

void
export_backup(const mtx::secret_storage::AesHmacSha2KeyDescription &keyDesc,
              const mtx::secret_storage::AesHmacSha2EncryptedData &secretData,
              const mtx::responses::backup::KeysBackup &backup)
{
    mtx::crypto::BinaryBuf decryptionKey;
    if (keyDesc.passphrase) {
        std::optional<mtx::crypto::BinaryBuf> temp;
        do {
            auto password = getpass("Enter Key Backup Password: ");
            temp          = mtx::crypto::key_from_passphrase(password, keyDesc);
        } while (!temp);
        decryptionKey = temp.value();
    } else {
        std::optional<mtx::crypto::BinaryBuf> temp;
        do {
            auto recoveryKey = getpass("Enter Key Backup Recovery Key: ");
            temp             = mtx::crypto::key_from_recoverykey(recoveryKey, keyDesc);
        } while (!temp);
        decryptionKey = temp.value();
    }

    // verify key
    using namespace mtx::crypto;

    auto decryptedSecret =
      decrypt(secretData, decryptionKey, mtx::secret_storage::secrets::megolm_backup_v1);

    if (decryptedSecret.empty()) {
        cerr << "Failed to get backup key from secret";
        return;
    }
    auto sessionDecryptionKey = to_binary_buf(base642bin(decryptedSecret));

    std::vector<ExportedSession> exported_sessions;

    for (const auto &[room_id, backup_sessions] : backup.rooms) {
        for (const auto &[session_id, s] : backup_sessions.sessions) {
            try {
                auto session = decrypt_session(s.session_data, sessionDecryptionKey);

                mtx::crypto::ExportedSession export_session{};
                export_session.algorithm           = session.algorithm;
                export_session.sender_key          = session.sender_key;
                export_session.session_id          = session_id;
                export_session.room_id             = room_id;
                export_session.session_key         = session.session_key;
                export_session.sender_claimed_keys = session.sender_claimed_keys;
                export_session.forwarding_curve25519_key_chain =
                  session.forwarding_curve25519_key_chain;

                exported_sessions.push_back(std::move(export_session));
            } catch (std::exception &e) {
                cerr << "Failed to decrypt session " << room_id << ":" << session_id << ":"
                     << e.what() << "\n";
                continue;
            }
            cerr << "Decrypted session " << room_id << ":" << session_id << "\n";
        }
    }

    auto encrypted_file = mtx::crypto::encrypt_exported_sessions(
      {exported_sessions}, getpass("Encryption password for export file:"));

    std::ofstream file;
    file.open("exported_sessions.txt");
    file << HEADER_LINE << "\n" << mtx::crypto::bin2base64(encrypted_file) << "\n" << TRAILER_LINE;

    file.close();
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
