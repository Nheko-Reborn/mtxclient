#include <atomic>

#include <boost/algorithm/string.hpp>

#include <gtest/gtest.h>

#include "olm/account.hh"

#include "client.hpp"
#include "crypto.hpp"

#include "mtx/requests.hpp"
#include "mtx/responses.hpp"
#include "variant.hpp"

#include "test_helpers.hpp"

using namespace mtx::client;
using namespace mtx::identifiers;
using namespace mtx::events::collections;
using namespace mtx::responses;

using namespace std;

mtx::requests::UploadKeys
generate_keys(std::shared_ptr<mtx::client::crypto::OlmClient> account)
{
        auto idks = account->identity_keys();
        account->generate_one_time_keys(1);
        auto otks = account->one_time_keys();

        return account->create_upload_keys_request(otks);
}

TEST(Encryption, UploadIdentityKeys)
{
        auto alice       = std::make_shared<Client>("localhost");
        auto olm_account = std::make_shared<mtx::client::crypto::OlmClient>();
        olm_account->create_new_account();

        alice->login("alice", "secret", [](const mtx::responses::Login &, RequestErr err) {
                check_error(err);
        });

        while (alice->access_token().empty())
                sleep();

        olm_account->set_user_id(alice->user_id().to_string());
        olm_account->set_device_id(alice->device_id());

        auto id_keys = olm_account->identity_keys();

        ASSERT_TRUE(id_keys.curve25519.size() > 10);
        ASSERT_TRUE(id_keys.curve25519.size() > 10);

        mtx::client::crypto::OneTimeKeys unused;
        auto request = olm_account->create_upload_keys_request(unused);

        // Make the request with the signed identity keys.
        alice->upload_keys(request, [](const mtx::responses::UploadKeys &res, RequestErr err) {
                check_error(err);
                EXPECT_EQ(res.one_time_key_counts.size(), 0);
        });

        alice->close();
}

TEST(Encryption, UploadOneTimeKeys)
{
        auto alice       = std::make_shared<Client>("localhost");
        auto olm_account = std::make_shared<mtx::client::crypto::OlmClient>();
        olm_account->create_new_account();

        alice->login("alice", "secret", [](const mtx::responses::Login &, RequestErr err) {
                check_error(err);
        });

        while (alice->access_token().empty())
                sleep();

        olm_account->set_user_id(alice->user_id().to_string());
        olm_account->set_device_id(alice->device_id());

        auto nkeys = olm_account->generate_one_time_keys(5);
        EXPECT_EQ(nkeys, 5);

        json otks = olm_account->one_time_keys();

        mtx::requests::UploadKeys req;

        // Create the proper structure for uploading.
        std::map<std::string, json> unsigned_keys;

        auto obj = otks.at("curve25519");
        for (auto it = obj.begin(); it != obj.end(); ++it)
                unsigned_keys["curve25519:" + it.key()] = it.value();

        req.one_time_keys = unsigned_keys;

        alice->upload_keys(req, [](const mtx::responses::UploadKeys &res, RequestErr err) {
                check_error(err);
                EXPECT_EQ(res.one_time_key_counts.size(), 1);
                EXPECT_EQ(res.one_time_key_counts.at("curve25519"), 5);
        });

        alice->close();
}

TEST(Encryption, UploadSignedOneTimeKeys)
{
        auto alice       = std::make_shared<Client>("localhost");
        auto olm_account = std::make_shared<mtx::client::crypto::OlmClient>();
        olm_account->create_new_account();

        alice->login("alice", "secret", [](const mtx::responses::Login &, RequestErr err) {
                check_error(err);
        });

        while (alice->access_token().empty())
                sleep();

        olm_account->set_user_id(alice->user_id().to_string());
        olm_account->set_device_id(alice->device_id());

        auto nkeys = olm_account->generate_one_time_keys(5);
        EXPECT_EQ(nkeys, 5);

        auto one_time_keys = olm_account->one_time_keys();

        mtx::requests::UploadKeys req;
        req.one_time_keys = olm_account->sign_one_time_keys(one_time_keys);

        alice->upload_keys(req, [nkeys](const mtx::responses::UploadKeys &res, RequestErr err) {
                check_error(err);
                EXPECT_EQ(res.one_time_key_counts.size(), 1);
                EXPECT_EQ(res.one_time_key_counts.at("signed_curve25519"), nkeys);
        });

        alice->close();
}

TEST(Encryption, UploadKeys)
{
        auto alice       = std::make_shared<Client>("localhost");
        auto olm_account = std::make_shared<mtx::client::crypto::OlmClient>();
        olm_account->create_new_account();

        alice->login("alice", "secret", [](const mtx::responses::Login &, RequestErr err) {
                check_error(err);
        });

        while (alice->access_token().empty())
                sleep();

        olm_account->set_user_id(alice->user_id().to_string());
        olm_account->set_device_id(alice->device_id());

        auto req = generate_keys(olm_account);

        alice->upload_keys(req, [](const mtx::responses::UploadKeys &res, RequestErr err) {
                check_error(err);
                EXPECT_EQ(res.one_time_key_counts.size(), 1);
                EXPECT_EQ(res.one_time_key_counts.at("signed_curve25519"), 1);
        });

        alice->close();
}

TEST(Encryption, QueryKeys)
{
        auto alice     = std::make_shared<Client>("localhost");
        auto alice_olm = std::make_shared<mtx::client::crypto::OlmClient>();

        auto bob     = std::make_shared<Client>("localhost");
        auto bob_olm = std::make_shared<mtx::client::crypto::OlmClient>();

        alice_olm->create_new_account();
        bob_olm->create_new_account();

        alice->login("alice", "secret", [](const mtx::responses::Login &, RequestErr err) {
                check_error(err);
        });

        bob->login(
          "bob", "secret", [](const mtx::responses::Login &, RequestErr err) { check_error(err); });

        while (alice->access_token().empty() || bob->access_token().empty())
                sleep();

        alice_olm->set_user_id(alice->user_id().to_string());
        alice_olm->set_device_id(alice->device_id());

        bob_olm->set_user_id(bob->user_id().to_string());
        bob_olm->set_device_id(bob->device_id());

        // Create and upload keys for both users.
        auto alice_req = ::generate_keys(alice_olm);
        auto bob_req   = ::generate_keys(bob_olm);

        // Validates that both upload requests are finished.
        atomic_int uploads(0);

        alice->upload_keys(alice_req,
                           [&uploads](const mtx::responses::UploadKeys &res, RequestErr err) {
                                   check_error(err);
                                   EXPECT_EQ(res.one_time_key_counts.size(), 1);
                                   EXPECT_EQ(res.one_time_key_counts.at("signed_curve25519"), 1);

                                   uploads += 1;
                           });

        bob->upload_keys(bob_req,
                         [&uploads](const mtx::responses::UploadKeys &res, RequestErr err) {
                                 check_error(err);
                                 EXPECT_EQ(res.one_time_key_counts.size(), 1);
                                 EXPECT_EQ(res.one_time_key_counts.at("signed_curve25519"), 1);

                                 uploads += 1;
                         });

        while (uploads != 2)
                sleep();

        atomic_int responses(0);

        // Each user is requests each other's keys.
        mtx::requests::QueryKeys alice_rk;
        alice_rk.device_keys[bob->user_id().to_string()] = {};
        alice->query_keys(
          alice_rk,
          [&responses, bob, bob_req](const mtx::responses::QueryKeys &res, RequestErr err) {
                  check_error(err);

                  ASSERT_TRUE(res.failures.size() == 0);

                  auto bob_devices = res.device_keys.at(bob->user_id().to_string());
                  ASSERT_TRUE(bob_devices.size() > 0);

                  auto dev_keys = bob_devices.at(bob->device_id());
                  EXPECT_EQ(dev_keys.user_id, bob->user_id().to_string());
                  EXPECT_EQ(dev_keys.device_id, bob->device_id());
                  EXPECT_EQ(dev_keys.keys, bob_req.device_keys.keys);
                  EXPECT_EQ(dev_keys.signatures, bob_req.device_keys.signatures);

                  responses += 1;
          });

        mtx::requests::QueryKeys bob_rk;
        bob_rk.device_keys[alice->user_id().to_string()] = {};
        bob->query_keys(
          bob_rk,
          [&responses, alice, alice_req](const mtx::responses::QueryKeys &res, RequestErr err) {
                  check_error(err);

                  ASSERT_TRUE(res.failures.size() == 0);

                  auto bob_devices = res.device_keys.at(alice->user_id().to_string());
                  ASSERT_TRUE(bob_devices.size() > 0);

                  auto dev_keys = bob_devices.at(alice->device_id());
                  EXPECT_EQ(dev_keys.user_id, alice->user_id().to_string());
                  EXPECT_EQ(dev_keys.device_id, alice->device_id());
                  EXPECT_EQ(dev_keys.keys, alice_req.device_keys.keys);
                  EXPECT_EQ(dev_keys.signatures, alice_req.device_keys.signatures);

                  responses += 1;
          });

        while (responses != 2)
                sleep();

        alice->close();
        bob->close();
}

TEST(Encryption, ClaimKeys)
{
        using namespace mtx::client::crypto;

        auto alice     = std::make_shared<Client>("localhost");
        auto alice_olm = std::make_shared<OlmClient>();

        alice_olm->create_new_account();
        alice->login("alice", "secret", check_login);

        auto bob     = std::make_shared<Client>("localhost");
        auto bob_olm = std::make_shared<OlmClient>();

        bob_olm->create_new_account();
        bob->login("bob", "secret", check_login);

        while (alice->access_token().empty() || bob->access_token().empty())
                sleep();

        alice_olm->set_user_id(alice->user_id().to_string());
        alice_olm->set_device_id(alice->device_id());

        bob_olm->set_user_id(bob->user_id().to_string());
        bob_olm->set_device_id(bob->device_id());

        atomic_bool uploaded(false);

        // Bob uploads his keys.
        auto bob_req = ::generate_keys(bob_olm);
        bob->upload_keys(bob_req,
                         [&uploaded](const mtx::responses::UploadKeys &res, RequestErr err) {
                                 check_error(err);
                                 EXPECT_EQ(res.one_time_key_counts.size(), 1);
                                 EXPECT_EQ(res.one_time_key_counts.at("signed_curve25519"), 1);
                                 uploaded = true;
                         });

        // Waiting for the upload to finish.
        while (!uploaded)
                sleep();

        // Alice retrieves bob's keys & claims one signed one-time key.
        mtx::requests::QueryKeys alice_rk;
        alice_rk.device_keys[bob->user_id().to_string()] = {};
        alice->query_keys(
          alice_rk,
          [alice_olm, alice, bob, bob_req](const mtx::responses::QueryKeys &res, RequestErr err) {
                  check_error(err);

                  auto bob_devices = res.device_keys.at(bob->user_id().to_string());
                  ASSERT_TRUE(bob_devices.size() > 0);

                  auto devices = {bob->device_id()};

                  // Retrieve the identity key for the current device.
                  auto bob_ed25519 =
                    bob_devices.at(bob->device_id()).keys.at("ed25519:" + bob->device_id());

                  alice->claim_keys(
                    bob->user_id(),
                    devices,
                    [alice_olm, bob, bob_req, bob_ed25519](const mtx::responses::ClaimKeys &res,
                                                           RequestErr err) {
                            check_error(err);

                            const auto user_id   = bob->user_id().to_string();
                            const auto device_id = bob->device_id();

                            // The device exists.
                            EXPECT_EQ(res.one_time_keys.size(), 1);
                            EXPECT_EQ(res.one_time_keys.at(user_id).size(), 1);

                            // The key is the one bob sent.
                            auto one_time_key = res.one_time_keys.at(user_id).at(device_id);
                            ASSERT_TRUE(one_time_key.is_object());

                            auto algo     = one_time_key.begin().key();
                            auto contents = one_time_key.begin().value();

                            EXPECT_EQ(bob_req.one_time_keys.at(algo), contents);

                            alice_olm->create_new_utility();

                            auto msg = json{{"key", contents.at("key").get<std::string>()}}.dump();
                            auto signature = contents.at("signatures")
                                               .at(user_id)
                                               .at("ed25519:" + device_id)
                                               .get<std::string>();

                            // Verify signature.
                            auto ret = olm_ed25519_verify(
                              alice_olm->utility(),
                              reinterpret_cast<const uint8_t *>(bob_ed25519.data()),
                              bob_ed25519.size(),
                              msg.data(),
                              msg.size(),
                              reinterpret_cast<uint8_t *>(&signature[0]),
                              signature.size());

                            EXPECT_EQ(std::string(olm_utility_last_error(alice_olm->utility())),
                                      "SUCCESS");
                            EXPECT_EQ(ret, 0);
                    });
          });

        alice->close();
        bob->close();
}

TEST(Encryption, KeyChanges)
{
        auto carl     = std::make_shared<Client>("localhost");
        auto carl_olm = std::make_shared<mtx::client::crypto::OlmClient>();
        carl_olm->create_new_account();

        carl->login("carl", "secret", [](const mtx::responses::Login &, RequestErr err) {
                check_error(err);
        });

        while (carl->access_token().empty())
                sleep();

        carl_olm->set_device_id(carl->device_id());
        carl_olm->set_user_id(carl->user_id().to_string());

        mtx::requests::CreateRoom req;
        carl->create_room(
          req, [carl, carl_olm](const mtx::responses::CreateRoom &, RequestErr err) {
                  check_error(err);

                  // Carl syncs to get the first next_batch token.
                  carl->sync(
                    "",
                    "",
                    false,
                    0,
                    [carl, carl_olm](const mtx::responses::Sync &res, RequestErr err) {
                            check_error(err);
                            const auto next_batch_token = res.next_batch;

                            auto key_req = ::generate_keys(carl_olm);

                            atomic_bool keys_uploaded(false);

                            // Changes his keys.
                            carl->upload_keys(
                              key_req,
                              [&keys_uploaded](const mtx::responses::UploadKeys &, RequestErr err) {
                                      check_error(err);
                                      keys_uploaded = true;
                              });

                            while (!keys_uploaded)
                                    sleep();

                            // The key changes should contain his username
                            // because of the key uploading.
                            carl->key_changes(
                              next_batch_token,
                              "",
                              [carl](const mtx::responses::KeyChanges &res, RequestErr err) {
                                      check_error(err);

                                      EXPECT_EQ(res.changed.size(), 1);
                                      EXPECT_EQ(res.left.size(), 0);

                                      EXPECT_EQ(res.changed.at(0), carl->user_id().to_string());
                              });
                    });
          });

        carl->close();
}

TEST(Encryption, EnableEncryption)
{
        auto bob  = make_shared<Client>("localhost");
        auto carl = make_shared<Client>("localhost");

        bob->login("bob", "secret", [](const Login &, RequestErr err) { check_error(err); });
        carl->login("carl", "secret", [](const Login &, RequestErr err) { check_error(err); });

        while (bob->access_token().empty() || carl->access_token().empty())
                sleep();

        atomic_int responses(0);
        mtx::identifiers::Room joined_room;

        mtx::requests::CreateRoom req;
        req.invite = {"@carl:localhost"};
        bob->create_room(
          req,
          [bob, carl, &responses, &joined_room](const mtx::responses::CreateRoom &res,
                                                RequestErr err) {
                  check_error(err);
                  joined_room = res.room_id;

                  bob->enable_encryption(
                    res.room_id, [&responses](const mtx::responses::EventId &, RequestErr err) {
                            check_error(err);
                            responses += 1;
                    });

                  carl->join_room(res.room_id,
                                  [&responses](const nlohmann::json &, RequestErr err) {
                                          check_error(err);
                                          responses += 1;
                                  });
          });

        while (responses != 2)
                sleep();

        carl->sync("", "", false, 0, [&joined_room](const Sync &res, RequestErr err) {
                check_error(err);

                auto events = res.rooms.join.at(joined_room.to_string()).timeline.events;

                using namespace mtx::events;

                int has_encryption = 0;
                for (const auto &e : events) {
                        if (mpark::holds_alternative<StateEvent<state::Encryption>>(e))
                                has_encryption = 1;
                }

                ASSERT_TRUE(has_encryption == 1);
        });

        bob->close();
        carl->close();
}

TEST(Encryption, CreateOutboundGroupSession)
{
        auto alice = make_shared<mtx::client::crypto::OlmClient>();
        auto bob   = make_shared<mtx::client::crypto::OlmClient>();

        alice->create_new_account();
        bob->create_new_account();

        bob->generate_one_time_keys(1);
        alice->generate_one_time_keys(1);

        auto outbound_session = alice->init_outbound_group_session();

        auto session_id  = mtx::client::crypto::session_id(outbound_session.get());
        auto session_key = mtx::client::crypto::session_key(outbound_session.get());
}

TEST(Encryption, OlmSessions)
{
        using namespace mtx::client::crypto;

        auto alice = std::make_shared<OlmClient>();
        alice->create_new_account();
        alice->generate_one_time_keys(1);

        auto bob = std::make_shared<OlmClient>();
        bob->create_new_account();
        bob->generate_one_time_keys(1);

        std::string alice_key          = alice->identity_keys().curve25519;
        std::string alice_one_time_key = alice->one_time_keys().curve25519.begin()->second;

        std::string bob_key          = bob->identity_keys().curve25519;
        std::string bob_one_time_key = bob->one_time_keys().curve25519.begin()->second;

        // Alice is preparing to send a pre-shared message to Bob by opening
        // a new 1-1 outbound session.
        auto alice_outbound_session = alice->create_outbound_session(bob_key, bob_one_time_key);

        // Alice encrypts the message using the current session.
        auto plaintext  = "Hello, Bob!";
        size_t msgtype  = olm_encrypt_message_type(alice_outbound_session.get());
        auto ciphertext = alice->encrypt_message(alice_outbound_session.get(), plaintext);

        EXPECT_EQ(msgtype, 0);

        // Bob creates an inbound session to receive Alice's message.
        auto bob_inbound_session = bob->create_inbound_session(ciphertext);

        // Bob validates that the message was meant for him.
        ASSERT_EQ(1, matches_inbound_session(bob_inbound_session.get(), ciphertext));

        // Bob validates that the message was sent from Alice.
        ASSERT_EQ(1,
                  matches_inbound_session_from(bob_inbound_session.get(), alice_key, ciphertext));

        // Bob validates that the message wasn't sent by someone else.
        ASSERT_EQ(0, matches_inbound_session_from(bob_inbound_session.get(), bob_key, ciphertext));

        // Bob decrypts the message
        auto ciphertext_str = std::string((char *)ciphertext.data(), ciphertext.size());
        auto decrypted = bob->decrypt_message(bob_inbound_session.get(), msgtype, ciphertext_str);

        auto body_str = std::string((char *)decrypted.data(), decrypted.size());
        ASSERT_EQ(body_str, "Hello, Bob!");
}
