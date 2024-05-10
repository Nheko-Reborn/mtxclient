#include <iostream>
#include <thread>

#include <unistd.h>

#include "mtx.hpp"
#include "mtxclient/crypto/client.hpp"
#include "mtxclient/http/client.hpp"
#include "mtxclient/http/errors.hpp"

namespace {
std::shared_ptr<mtx::http::Client> client = nullptr;
}

using mtx::http::RequestErr;

void
print_errors(mtx::http::RequestErr err)
{
    if (!err->parse_error.empty())
        std::cerr << err->parse_error;
    if (!err->matrix_error.error.empty())
        std::cerr << err->matrix_error.error;
    if (err->error_code)
        std::cerr << err->error_code;
    if (err->status_code)
        std::cerr << err->status_code;
    std::cerr << "\n";
}

void
verify_secret_storage(const std::string &secret_key_id,
                      const mtx::responses::backup::BackupVersion &backupVersion,
                      const mtx::secret_storage::AesHmacSha2KeyDescription &keyDesc,
                      const mtx::secret_storage::AesHmacSha2EncryptedData &okbSecret,
                      const mtx::crypto::CrossSigningKeys &masterkey,
                      const mtx::secret_storage::AesHmacSha2EncryptedData &masterKeySecret)
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

    std::cout << "Successfully decrypted secret storage key\n";

    using namespace mtx::crypto;

    // verify okb key
    auto decryptedOkbSecret =
      decrypt(okbSecret, decryptionKey, mtx::secret_storage::secrets::megolm_backup_v1);

    if (decryptedOkbSecret.empty()) {
        std::cerr << "Failed to get backup key from secret";
        client->logout([](mtx::responses::Logout, RequestErr) {});
        return;
    }

    std::cout << "Successfully decrypted online key backup key\n";
    {
        auto privateKey = to_binary_buf(base642bin(decryptedOkbSecret));

        auto pubkey = CURVE25519_public_key_from_private(privateKey);

        std::cout << "OKB Public key: " << pubkey << "\n";

        auto auth_data = nlohmann::json::parse(backupVersion.auth_data);
        if (auth_data["public_key"].get<std::string>() != pubkey) {
            std::cerr << "OKB public key does not match current online key backup, aborting...";
            client->logout([](mtx::responses::Logout, RequestErr) {});
            return;
        }
    }

    // verify masterkey key
    auto decryptedMasterKeySecret =
      decrypt(masterKeySecret, decryptionKey, mtx::secret_storage::secrets::cross_signing_master);

    if (decryptedMasterKeySecret.empty()) {
        std::cerr << "Failed to get master key from secret";
        client->logout([](mtx::responses::Logout, RequestErr) {});
        return;
    }
    std::cout << "Successfully decrypted master key\n";

    auto mk = mtx::crypto::PkSigning::from_seed(decryptedMasterKeySecret);
    std::cout << "Master key: " << mk.public_key() << "\n";

    if (mk.public_key() != masterkey.keys.begin()->second) {
        std::cerr << "Master key from SSSS does not match your current master key, aborting...";
        client->logout([](mtx::responses::Logout, RequestErr) {});
        return;
    }

    if (!keyDesc.signatures.contains(client->user_id().to_string()) ||
        !keyDesc.signatures.at(client->user_id().to_string())
           .contains("ed25519:" + mk.public_key())) {
        std::cout << "Your secret storage key is not signed, do you want to sign it now? Type 'y' "
                     "to proceed:\n";
        std::string response;
        std::getline(std::cin, response);
        if (response == "y") {
            std::cout << "Signing secret storage key...\n";

            nlohmann::json j = keyDesc;
            j.erase("signatures");

            auto newKeyDesc = keyDesc;
            newKeyDesc.signatures[client->user_id().to_string()]["ed25519:" + mk.public_key()] =
              mk.sign(j.dump());
            std::cout << "Uploading new key descriptions for " << secret_key_id << ":\n"
                      << nlohmann::json(newKeyDesc).dump(4) << "\n";
            client->upload_secret_storage_key(
              secret_key_id, newKeyDesc, [](mtx::http::RequestErr err) {
                  if (err) {
                      std::cerr << "Failed to update the secret storage key signatures.";
                      print_errors(err);
                  } else {
                      std::cout << "Uploaded secret storage key signatures\n";
                  }
              });

            std::this_thread::sleep_for(std::chrono::seconds(1));
        }
    }

    std::cout << "Do you want to set " << secret_key_id
              << " as your default key? Type 'y' to proceed:\n";
    std::string response;
    std::getline(std::cin, response);
    if (response == "y") {
        std::cout << "Setting default key...\n";
        client->set_secret_storage_default_key(secret_key_id, [](mtx::http::RequestErr err) {
            if (err) {
                std::cerr << "Failed to update the default secret storage key";
                print_errors(err);
            } else {
                std::cout << "Set default key\n";
            }
        });

        std::this_thread::sleep_for(std::chrono::seconds(1));
    }

    client->logout([](mtx::responses::Logout, RequestErr) {});
}

void
login_handler(const mtx::responses::Login &res, mtx::http::RequestErr err)
{
    if (err) {
        std::cout << "There was an error during login: " << err->matrix_error.error << "\n";
        return;
    }

    std::cout << "Logged in as: " << res.user_id.to_string()
              << "\nDiscovering cross-signing setup...\n";

    client->set_access_token(res.access_token);

    client->backup_version([](const mtx::responses::backup::BackupVersion &backup_version,
                              RequestErr err) {
        if (err) {
            std::cerr << "Error fetching the backup version: ";
            print_errors(err);
            client->logout([](mtx::responses::Logout, RequestErr) {});
            return;
        }

        if (backup_version.algorithm != mtx::responses::backup::megolm_backup_v1) {
            std::cerr << "Incompatible backup algorithm: " << backup_version.algorithm << "\n";
            client->logout([](mtx::responses::Logout, RequestErr) {});
            return;
        }

        std::cout << "Found online key backup:\n" << nlohmann::json(backup_version).dump(4) << "\n";

        client->secret_storage_secret(
          mtx::secret_storage::secrets::megolm_backup_v1,
          [backup_version](mtx::secret_storage::Secret secret, RequestErr err) {
              if (err) {
                  std::cerr << "Error fetching the backup secret: ";
                  print_errors(err);
                  client->logout([](mtx::responses::Logout, RequestErr) {});
                  return;
              }

              std::vector<std::string> encrypted_by;

              for (const auto &k : secret.encrypted) {
                  encrypted_by.push_back(k.first);
              }

              std::cout << "Found online key backup encryption key: "
                        << fmt::format("{}", fmt::join(encrypted_by, ", ")) << "\n";

              if (encrypted_by.empty()) {
                  std::cerr << "Not encrypted by any key, aborting...";
                  client->logout([](mtx::responses::Logout, RequestErr) {});
                  return;
              }

              auto okb_key_id = encrypted_by.front();

              client->secret_storage_secret(
                mtx::secret_storage::secrets::cross_signing_master,
                [backup_version, okbSecret = secret.encrypted.begin()->second, okb_key_id](
                  mtx::secret_storage::Secret secret, RequestErr err) {
                    if (err) {
                        std::cerr << "Error fetching the master secret: ";
                        print_errors(err);
                        client->logout([](mtx::responses::Logout, RequestErr) {});
                        return;
                    }

                    std::vector<std::string> encrypted_by;

                    for (const auto &k : secret.encrypted) {
                        encrypted_by.push_back(k.first);
                    }

                    std::cout << "Found master key encryption key: "
                              << fmt::format("{}", fmt::join(encrypted_by, ", ")) << "\n";

                    if (encrypted_by.empty()) {
                        std::cerr << "Not encrypted by any key, aborting...";
                        client->logout([](mtx::responses::Logout, RequestErr) {});
                        return;
                    }

                    if (okb_key_id != encrypted_by.front()) {
                        std::cerr << "Master key and backup key encrypted with different keys! "
                                     "Aborting...";
                        client->logout([](mtx::responses::Logout, RequestErr) {});
                        return;
                    }

                    client->secret_storage_key(
                      okb_key_id,
                      [okb_key_id,
                       backup_version,
                       okbSecret,
                       masterKeySecret = secret.encrypted.begin()->second](
                        const mtx::secret_storage::AesHmacSha2KeyDescription &keyDesc,
                        RequestErr err) {
                          if (err) {
                              std::cerr << "Error fetching the backup key "
                                           "description: ";
                              print_errors(err);
                              client->logout([](mtx::responses::Logout, RequestErr) {});
                              return;
                          }

                          std::cout << "Found secret for online key backup:\n"
                                    << nlohmann::json(keyDesc).dump(4) << "\n";

                          if (keyDesc.signatures.empty()) {
                              std::cerr
                                << "Secret storage key has no signatures, which is weird!\n";
                          }

                          mtx::requests::QueryKeys keyQuery;
                          keyQuery.device_keys[client->user_id().to_string()] = {};
                          client->query_keys(
                            keyQuery,
                            [okb_key_id, backup_version, okbSecret, masterKeySecret, keyDesc](
                              const mtx::responses::QueryKeys &userKeys, RequestErr err) {
                                if (err) {
                                    std::cerr << "Failed to query own device keys:";
                                    print_errors(err);
                                    client->logout([](mtx::responses::Logout, RequestErr) {});
                                    return;
                                }

                                if (userKeys.master_keys.empty()) {
                                    std::cerr << "NO MASTER KEY?!?";
                                    client->logout([](mtx::responses::Logout, RequestErr) {});
                                    return;
                                }

                                auto masterkey = userKeys.master_keys.begin()->second;
                                std::cout << "Your master key is:\n"
                                          << nlohmann::json(masterkey).dump(4) << "\n";

                                verify_secret_storage(okb_key_id,
                                                      backup_version,
                                                      keyDesc,
                                                      okbSecret,
                                                      masterkey,
                                                      masterKeySecret);
                            });
                      });
                });
          });
    });
}

int
main()
{
    std::string username, server, password;

    std::cout << "Username: ";
    std::getline(std::cin, username);

    std::cout << "HomeServer: ";
    std::getline(std::cin, server);

    password = getpass("Password: ");

    client = std::make_shared<mtx::http::Client>(server);
    client->login(username, password, &login_handler);
    client->close();

    return 0;
}
