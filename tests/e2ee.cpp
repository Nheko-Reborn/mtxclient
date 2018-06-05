#include <atomic>

#include <boost/algorithm/string.hpp>

#include <gtest/gtest.h>

#include "olm/account.hh"

#include "mtxclient/crypto/client.hpp"
#include "mtxclient/http/client.hpp"

#include "mtx/requests.hpp"
#include "mtx/responses.hpp"
#include "variant.hpp"

#include "test_helpers.hpp"

using namespace mtx::client;
using namespace mtx::http;
using namespace mtx::crypto;

using namespace mtx::identifiers;
using namespace mtx::events::collections;
using namespace mtx::responses;

using namespace std;

struct OlmCipherContent
{
        std::string body;
        uint8_t type;
};

inline void
from_json(const nlohmann::json &obj, OlmCipherContent &msg)
{
        msg.body = obj.at("body");
        msg.type = obj.at("type");
}

struct OlmMessage
{
        std::string sender_key;
        std::string sender;

        using RecipientKey = std::string;
        std::map<RecipientKey, OlmCipherContent> ciphertext;
};

constexpr auto OLM_ALGO = "m.olm.v1.curve25519-aes-sha2";

inline void
from_json(const nlohmann::json &obj, OlmMessage &msg)
{
        if (obj.at("type") != "m.room.encrypted") {
                throw std::invalid_argument("invalid type for olm message");
        }

        if (obj.at("content").at("algorithm") != OLM_ALGO)
                throw std::invalid_argument("invalid algorithm for olm message");

        msg.sender     = obj.at("sender");
        msg.sender_key = obj.at("content").at("sender_key");
        msg.ciphertext =
          obj.at("content").at("ciphertext").get<std::map<std::string, OlmCipherContent>>();
}

mtx::requests::UploadKeys
generate_keys(std::shared_ptr<mtx::crypto::OlmClient> account)
{
        auto idks = account->identity_keys();
        account->generate_one_time_keys(1);
        auto otks = account->one_time_keys();

        return account->create_upload_keys_request(otks);
}

TEST(Encryption, UploadIdentityKeys)
{
        auto alice       = std::make_shared<Client>("localhost");
        auto olm_account = std::make_shared<mtx::crypto::OlmClient>();
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

        mtx::crypto::OneTimeKeys unused;
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
        auto olm_account = std::make_shared<mtx::crypto::OlmClient>();
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
        auto olm_account = std::make_shared<mtx::crypto::OlmClient>();
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
        auto olm_account = std::make_shared<mtx::crypto::OlmClient>();
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
        auto alice_olm = std::make_shared<mtx::crypto::OlmClient>();

        auto bob     = std::make_shared<Client>("localhost");
        auto bob_olm = std::make_shared<mtx::crypto::OlmClient>();

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
        using namespace mtx::crypto;

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
        bob_olm->generate_one_time_keys(1);
        bob->upload_keys(bob_olm->create_upload_keys_request(),
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
          alice_rk, [alice_olm, alice, bob](const mtx::responses::QueryKeys &res, RequestErr err) {
                  check_error(err);

                  auto bob_devices = res.device_keys.at(bob->user_id().to_string());
                  ASSERT_TRUE(bob_devices.size() > 0);

                  auto devices = {bob->device_id()};

                  // Retrieve the identity key for the current device.
                  auto bob_ed25519 =
                    bob_devices.at(bob->device_id()).keys.at("ed25519:" + bob->device_id());

                  const auto current_device = bob_devices.at(bob->device_id());

                  // Verify signature.
                  ASSERT_TRUE(verify_identity_signature(json(current_device),
                                                        DeviceId(bob->device_id()),
                                                        UserId(bob->user_id().to_string())));

                  alice->claim_keys(bob->user_id().to_string(),
                                    devices,
                                    [alice_olm, bob, bob_ed25519](
                                      const mtx::responses::ClaimKeys &res, RequestErr err) {
                                            check_error(err);

                                            const auto user_id   = bob->user_id().to_string();
                                            const auto device_id = bob->device_id();

                                            // The device exists.
                                            EXPECT_EQ(res.one_time_keys.size(), 1);
                                            EXPECT_EQ(res.one_time_keys.at(user_id).size(), 1);

                                            // The key is the one bob sent.
                                            auto one_time_key =
                                              res.one_time_keys.at(user_id).at(device_id);
                                            ASSERT_TRUE(one_time_key.is_object());

                                            auto algo     = one_time_key.begin().key();
                                            auto contents = one_time_key.begin().value();
                                    });
          });

        alice->close();
        bob->close();
}

TEST(Encryption, KeyChanges)
{
        auto carl     = std::make_shared<Client>("localhost");
        auto carl_olm = std::make_shared<mtx::crypto::OlmClient>();
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
                  SyncOpts opts;
                  opts.timeout = 0;
                  carl->sync(
                    opts, [carl, carl_olm](const mtx::responses::Sync &res, RequestErr err) {
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
                    res.room_id.to_string(),
                    [&responses](const mtx::responses::EventId &, RequestErr err) {
                            check_error(err);
                            responses += 1;
                    });

                  carl->join_room(res.room_id.to_string(),
                                  [&responses](const nlohmann::json &, RequestErr err) {
                                          check_error(err);
                                          responses += 1;
                                  });
          });

        while (responses != 2)
                sleep();

        SyncOpts opts;
        opts.timeout = 0;
        carl->sync(opts, [&joined_room](const Sync &res, RequestErr err) {
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
        auto alice = make_shared<mtx::crypto::OlmClient>();
        auto bob   = make_shared<mtx::crypto::OlmClient>();

        alice->create_new_account();
        bob->create_new_account();

        bob->generate_one_time_keys(1);
        alice->generate_one_time_keys(1);

        auto outbound_session = alice->init_outbound_group_session();

        auto session_id  = mtx::crypto::session_id(outbound_session.get());
        auto session_key = mtx::crypto::session_key(outbound_session.get());
}

TEST(Encryption, OlmSessions)
{
        using namespace mtx::crypto;

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
        auto plaintext      = "Hello, Bob!";
        size_t msgtype      = olm_encrypt_message_type(alice_outbound_session.get());
        auto ciphertext     = alice->encrypt_message(alice_outbound_session.get(), plaintext);
        auto ciphertext_str = std::string((char *)ciphertext.data(), ciphertext.size());

        EXPECT_EQ(msgtype, 0);

        // Bob creates an inbound session to receive Alice's message.
        auto bob_inbound_session = bob->create_inbound_session(ciphertext_str);

        // Bob validates that the message was meant for him.
        ASSERT_EQ(1, matches_inbound_session(bob_inbound_session.get(), ciphertext_str));

        // Bob validates that the message was sent from Alice.
        ASSERT_EQ(
          1, matches_inbound_session_from(bob_inbound_session.get(), alice_key, ciphertext_str));

        // Bob validates that the message wasn't sent by someone else.
        ASSERT_EQ(0,
                  matches_inbound_session_from(bob_inbound_session.get(), bob_key, ciphertext_str));

        // Bob decrypts the message
        auto decrypted = bob->decrypt_message(bob_inbound_session.get(), msgtype, ciphertext_str);

        auto body_str = std::string((char *)decrypted.data(), decrypted.size());
        ASSERT_EQ(body_str, plaintext);
}

TEST(Encryption, MegolmSessions)
{
        auto alice = std::make_shared<OlmClient>();
        alice->create_new_account();
        alice->generate_one_time_keys(1);

        auto bob = std::make_shared<OlmClient>();
        bob->create_new_account();
        bob->generate_one_time_keys(1);

        // Alice wants to send an encrypted megolm message to Bob.
        const std::string secret_message = "Hey, Bob!";

        // Alice creates an outbound megolm session that will be used by both parties.
        auto outbound_megolm_session = alice->init_outbound_group_session();
        auto msg_index = olm_outbound_group_session_message_index(outbound_megolm_session.get());
        ASSERT_EQ(msg_index, 0);

        // Alice extracts the session id & session key so she can share them with Bob.
        const auto session_id  = mtx::crypto::session_id(outbound_megolm_session.get());
        const auto session_key = mtx::crypto::session_key(outbound_megolm_session.get());

        // Encrypt the message using megolm.
        auto encrypted_secret_message =
          alice->encrypt_group_message(outbound_megolm_session.get(), secret_message);

        msg_index = olm_outbound_group_session_message_index(outbound_megolm_session.get());
        ASSERT_EQ(msg_index, 1);

        // First she will create an outbound olm session so she can share the session data.
        // Alice will need Bob's curve25519 key and one claimed one time key.
        auto outbound_olm_session = alice->create_outbound_session(
          bob->identity_keys().curve25519, bob->one_time_keys().curve25519.begin()->second);
        const auto msg_type = olm_encrypt_message_type(outbound_olm_session.get());

        // Plaintext version of the session data to be shared.
        const auto session_data = json{{"session_id", session_id}, {"session_key", session_key}};

        // Alice encrypts the session data using olm.
        const auto encrypted_session_data =
          alice->encrypt_message(outbound_olm_session.get(), session_data.dump());
        const auto encrypted_session_data_str =
          std::string((char *)encrypted_session_data.data(), encrypted_session_data.size());

        //
        // Alice sends the olm & megolm messages to Bob ...
        //

        // Bob creates an inbound olm session to receive the session data.
        auto inbound_olm_session = bob->create_inbound_session(encrypted_session_data);

        // and validates that the message was indeed from Alice.
        ASSERT_EQ(1,
                  matches_inbound_session_from(inbound_olm_session.get(),
                                               alice->identity_keys().curve25519,
                                               encrypted_session_data_str));

        // Bob decrypts the encrypted olm message.
        auto plaintext_session_data =
          bob->decrypt_message(inbound_olm_session.get(), msg_type, encrypted_session_data_str);
        auto session_str_data = json::parse(
          std::string((char *)plaintext_session_data.data(), plaintext_session_data.size()));

        // Validate that the output matches the input.
        ASSERT_EQ(session_id, session_str_data.at("session_id").get<std::string>());
        ASSERT_EQ(session_key, session_str_data.at("session_key").get<std::string>());

        // Bob will use the session_key to create an inbound megolm session.
        // The session_id will be used to map future messages to this session.
        auto inbound_megolm_session = bob->init_inbound_group_session(session_key);

        // Bob can finally decrypt Alice's original message.
        auto ciphertext =
          std::string((char *)encrypted_secret_message.data(), encrypted_secret_message.size());
        auto bob_plaintext = bob->decrypt_group_message(inbound_megolm_session.get(), ciphertext);

        auto output_str = std::string((char *)bob_plaintext.data.data(), bob_plaintext.data.size());
        ASSERT_EQ(output_str, secret_message);
}

TEST(Encryption, OlmRoomKeyEncryption)
{
        // Alice wants to use olm to send data to Bob.
        auto alice_olm  = std::make_shared<OlmClient>();
        auto alice_http = std::make_shared<Client>("localhost");
        alice_olm->create_new_account();
        alice_olm->generate_one_time_keys(10);

        auto bob_olm  = std::make_shared<OlmClient>();
        auto bob_http = std::make_shared<Client>("localhost");
        bob_olm->create_new_account();
        bob_olm->generate_one_time_keys(10);

        alice_http->login("alice", "secret", &check_login);
        bob_http->login("bob", "secret", &check_login);

        WAIT_UNTIL(!bob_http->access_token().empty() && !alice_http->access_token().empty())

        bob_olm->set_user_id(bob_http->user_id().to_string());
        bob_olm->set_device_id(bob_http->device_id());
        alice_olm->set_user_id(alice_http->user_id().to_string());
        alice_olm->set_device_id(alice_http->device_id());

        // Both users upload their identity & one time keys
        atomic_int uploads(0);
        auto upload_cb = [&uploads](const mtx::responses::UploadKeys &res, RequestErr err) {
                check_error(err);
                EXPECT_EQ(res.one_time_key_counts.size(), 1);
                EXPECT_EQ(res.one_time_key_counts.at("signed_curve25519"), 10);
                uploads += 1;
        };

        alice_http->upload_keys(alice_olm->create_upload_keys_request(), upload_cb);
        bob_http->upload_keys(bob_olm->create_upload_keys_request(), upload_cb);

        WAIT_UNTIL(uploads == 2)

        atomic_bool request_finished(false);
        std::string bob_ed25519, bob_curve25519, bob_otk;

        // Alice needs Bob's ed25519 device key.
        mtx::requests::QueryKeys query;
        query.device_keys[bob_http->user_id().to_string()] = {};
        alice_http->query_keys(query,
                               [&request_finished, &bob_ed25519, &bob_curve25519, bob = bob_http](
                                 const mtx::responses::QueryKeys &res, RequestErr err) {
                                       check_error(err);

                                       const auto device_id = bob->device_id();
                                       const auto user_id   = bob->user_id().to_string();
                                       const auto devices   = res.device_keys.at(user_id);

                                       assert(devices.find(device_id) != devices.end());

                                       bob_ed25519 =
                                         devices.at(device_id).keys.at("ed25519:" + device_id);
                                       bob_curve25519 =
                                         devices.at(device_id).keys.at("curve25519:" + device_id);

                                       request_finished = true;
                               });

        WAIT_UNTIL(request_finished);

        // Alice needs one of Bob's one time keys.
        request_finished = false;
        alice_http->claim_keys(bob_http->user_id().to_string(),
                               {bob_http->device_id()},
                               [&bob_otk, bob = bob_http, &request_finished](
                                 const mtx::responses::ClaimKeys &res, RequestErr err) {
                                       check_error(err);

                                       const auto user_id   = bob->user_id().to_string();
                                       const auto device_id = bob->device_id();

                                       auto retrieved_devices = res.one_time_keys.at(user_id);
                                       for (const auto &device : retrieved_devices) {
                                               if (device.first == device_id) {
                                                       bob_otk = device.second.begin()->at("key");
                                                       break;
                                               }
                                       }

                                       request_finished = true;
                               });

        WAIT_UNTIL(request_finished);

        EXPECT_EQ(bob_ed25519, bob_olm->identity_keys().ed25519);
        EXPECT_EQ(bob_curve25519, bob_olm->identity_keys().curve25519);
        EXPECT_EQ(bob_otk, bob_olm->one_time_keys().curve25519.begin()->second);

        constexpr auto SECRET_TEXT = "Hello Bob!";

        // Alice create m.room.key request
        json payload  = json{{"secret", SECRET_TEXT}};
        auto room_key = alice_olm->create_room_key_event(
          UserId("@bob:localhost"), bob_olm->identity_keys().ed25519, payload);

        // Alice creates an outbound session.
        auto out_session = alice_olm->create_outbound_session(bob_curve25519, bob_otk);
        auto device_msg  = alice_olm->create_olm_encrypted_content(
          out_session.get(), room_key.dump(), bob_curve25519);

        // Finally sends the olm encrypted message to Bob's device.
        atomic_bool is_sent(false);
        json body{
          {"messages", {{bob_http->user_id().to_string(), {{bob_http->device_id(), device_msg}}}}}};
        alice_http->send_to_device("m.room.encrypted", body, [&is_sent](RequestErr err) {
                check_error(err);
                is_sent = true;
        });

        WAIT_UNTIL(is_sent)

        SyncOpts opts;
        opts.timeout = 0;
        bob_http->sync(
          opts, [bob = bob_olm, SECRET_TEXT](const mtx::responses::Sync &res, RequestErr err) {
                  check_error(err);

                  assert(!res.to_device.empty());
                  assert(res.to_device.size() == 1);

                  OlmMessage olm_msg = res.to_device[0];
                  auto cipher        = olm_msg.ciphertext.begin();

                  EXPECT_EQ(cipher->first, bob->identity_keys().curve25519);

                  const auto msg_body = cipher->second.body;
                  const auto msg_type = cipher->second.type;

                  assert(msg_type == 0);

                  auto inbound_session = bob->create_inbound_session(msg_body);
                  auto ok              = matches_inbound_session_from(
                    inbound_session.get(), olm_msg.sender_key, msg_body);
                  assert(ok == true);

                  auto output = bob->decrypt_message(inbound_session.get(), msg_type, msg_body);

                  // Parsing the original plaintext json object.
                  auto plaintext = json::parse(std::string((char *)output.data(), output.size()));
                  std::string secret = plaintext.at("content").at("secret");

                  ASSERT_EQ(secret, SECRET_TEXT);
          });

        alice_http->close();
        bob_http->close();
}

TEST(Encryption, PickleAccount)
{
        auto alice = std::make_shared<OlmClient>();
        alice->create_new_account();
        alice->generate_one_time_keys(10);

        auto alice_pickled = pickle<AccountObject>(alice->account(), "secret");

        auto bob = std::make_shared<OlmClient>();
        bob->restore_account(alice_pickled, "secret");

        EXPECT_EQ(json(bob->identity_keys()).dump(), json(alice->identity_keys()).dump());
        EXPECT_EQ(json(bob->one_time_keys()).dump(), json(alice->one_time_keys()).dump());

        auto carl = std::make_shared<OlmClient>();

        // BAD_ACCOUNT_KEY
        EXPECT_THROW(carl->restore_account(alice_pickled, "another_secret"), olm_exception);
}

TEST(Encryption, PickleOlmSessions)
{
        auto alice = std::make_shared<OlmClient>();
        alice->create_new_account();

        auto bob = std::make_shared<OlmClient>();
        bob->create_new_account();
        bob->generate_one_time_keys(1);

        std::string bob_key          = bob->identity_keys().curve25519;
        std::string bob_one_time_key = bob->one_time_keys().curve25519.begin()->second;

        auto outbound_session = alice->create_outbound_session(bob_key, bob_one_time_key);

        auto plaintext      = "Hello, Bob!";
        size_t msgtype      = olm_encrypt_message_type(outbound_session.get());
        auto ciphertext     = alice->encrypt_message(outbound_session.get(), plaintext);
        auto ciphertext_str = std::string((char *)ciphertext.data(), ciphertext.size());

        EXPECT_EQ(msgtype, 0);

        auto saved_outbound_session    = pickle<SessionObject>(outbound_session.get(), "wat");
        auto restored_outbound_session = unpickle<SessionObject>(saved_outbound_session, "wat");

        EXPECT_THROW(unpickle<SessionObject>(saved_outbound_session, "another_secret"),
                     olm_exception);

        msgtype = olm_encrypt_message_type(restored_outbound_session.get());
        EXPECT_EQ(msgtype, 0);

        auto restored_ciphertext =
          alice->encrypt_message(restored_outbound_session.get(), plaintext);
        auto restored_ciphertext_str =
          std::string((char *)restored_ciphertext.data(), restored_ciphertext.size());

        auto inbound_session          = bob->create_inbound_session(ciphertext_str);
        auto saved_inbound_session    = pickle<SessionObject>(inbound_session.get(), "woot");
        auto restored_inbound_session = unpickle<SessionObject>(saved_inbound_session, "woot");

        EXPECT_THROW(unpickle<SessionObject>(saved_inbound_session, "another_secret"),
                     olm_exception);

        ASSERT_EQ(1, matches_inbound_session(inbound_session.get(), ciphertext_str));
        ASSERT_EQ(1, matches_inbound_session(inbound_session.get(), restored_ciphertext_str));
        ASSERT_EQ(1,
                  matches_inbound_session(restored_inbound_session.get(), restored_ciphertext_str));
        ASSERT_EQ(1, matches_inbound_session(restored_inbound_session.get(), ciphertext_str));

        auto d1 = bob->decrypt_message(inbound_session.get(), msgtype, ciphertext_str);
        auto d2 = bob->decrypt_message(restored_inbound_session.get(), msgtype, ciphertext_str);
        auto d3 = bob->decrypt_message(inbound_session.get(), msgtype, restored_ciphertext_str);
        auto d4 =
          bob->decrypt_message(restored_inbound_session.get(), msgtype, restored_ciphertext_str);

        EXPECT_EQ(d1, d2);
        EXPECT_EQ(d2, d3);
        EXPECT_EQ(d3, d4);
        EXPECT_EQ(d1, d4);
        EXPECT_EQ(d2, d4);

        EXPECT_EQ(std::string((char *)d1.data(), d1.size()), "Hello, Bob!");
}

TEST(Encryption, PickleMegolmSessions)
{
        // Outbound Session
        auto alice = make_shared<mtx::crypto::OlmClient>();
        alice->create_new_account();

        auto outbound_session = alice->init_outbound_group_session();

        const auto original_session_id  = mtx::crypto::session_id(outbound_session.get());
        const auto original_session_key = mtx::crypto::session_key(outbound_session.get());

        auto saved_session = pickle<OutboundSessionObject>(outbound_session.get(), "secret");
        auto restored_outbound_session = unpickle<OutboundSessionObject>(saved_session, "secret");

        const auto restored_session_id  = mtx::crypto::session_id(restored_outbound_session.get());
        const auto restored_session_key = mtx::crypto::session_key(restored_outbound_session.get());

        EXPECT_EQ(original_session_id, restored_session_id);
        EXPECT_EQ(original_session_key, restored_session_key);

        // BAD_ACCOUNT_KEY
        EXPECT_THROW(unpickle<OutboundSessionObject>(saved_session, "another_secret"),
                     olm_exception);

        const auto SECRET = "Hello World!";

        auto encrypted  = alice->encrypt_group_message(outbound_session.get(), SECRET);
        auto ciphertext = std::string((char *)encrypted.data(), encrypted.size());

        // Inbound Session
        auto inbound_session = alice->init_inbound_group_session(original_session_key);
        auto plaintext       = alice->decrypt_group_message(inbound_session.get(), ciphertext);

        saved_session = pickle<InboundSessionObject>(inbound_session.get(), "secret");

        auto restored_inbound_session = unpickle<InboundSessionObject>(saved_session, "secret");
        auto restored_plaintext =
          alice->decrypt_group_message(restored_inbound_session.get(), ciphertext);

        EXPECT_EQ(
          std::string((char *)plaintext.data.data(), plaintext.data.size()),
          std::string((char *)restored_plaintext.data.data(), restored_plaintext.data.size()));

        EXPECT_EQ(std::string((char *)plaintext.data.data(), plaintext.data.size()), SECRET);
}

TEST(Encryption, DISABLED_HandleRoomKeyEvent) {}
TEST(Encryption, DISABLED_HandleRoomKeyRequestEvent) {}
TEST(Encryption, DISABLED_HandleNewDevices) {}
TEST(Encryption, DISABLED_HandleLeftDevices) {}

TEST(Encryption, DISABLED_SendEncryptedMessageWithMegolm) {}
TEST(Encryption, DISABLED_RotateMegolmSession) {}
