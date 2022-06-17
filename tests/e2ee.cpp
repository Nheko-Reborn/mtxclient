#include <atomic>

#include <gtest/gtest.h>

#include <nlohmann/json.hpp>

#include "mtxclient/crypto/client.hpp"
#include "mtxclient/crypto/types.hpp"
#include "mtxclient/http/client.hpp"

#include "mtx/requests.hpp"
#include "mtx/responses.hpp"

#include "test_helpers.hpp"

using namespace mtx::client;
using namespace mtx::http;
using namespace mtx::crypto;

using namespace mtx::identifiers;
using namespace mtx::events;
using namespace mtx::events::msg;
using namespace mtx::events::collections;
using namespace mtx::responses;

using namespace std;

using namespace nlohmann;

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

    msg.sender     = obj.at("sender").get<std::string>();
    msg.sender_key = obj.at("content").at("sender_key").get<std::string>();
    msg.ciphertext =
      obj.at("content").at("ciphertext").get<std::map<std::string, OlmCipherContent>>();
}

mtx::requests::UploadKeys
generate_keys(std::shared_ptr<mtx::crypto::OlmClient> account)
{
    auto idks = account->identity_keys();
    account->generate_one_time_keys(1);
    auto otks = account->one_time_keys();

    return account->create_upload_keys_request(otks, {});
}

TEST(Encryption, UploadIdentityKeys)
{
    auto alice       = make_test_client();
    auto olm_account = std::make_shared<mtx::crypto::OlmClient>();

    EXPECT_THROW(olm_account->identity_keys(), olm_exception);

    olm_account->create_new_account();

    alice->login(
      "alice", "secret", [](const mtx::responses::Login &, RequestErr err) { check_error(err); });

    while (alice->access_token().empty())
        sleep();

    olm_account->set_user_id(alice->user_id().to_string());
    olm_account->set_device_id(alice->device_id());

    auto id_keys = olm_account->identity_keys();

    ASSERT_TRUE(id_keys.curve25519.size() > 10);
    ASSERT_TRUE(id_keys.curve25519.size() > 10);

    mtx::crypto::OneTimeKeys unused;
    auto request = olm_account->create_upload_keys_request(unused, {});

    // Make the request with the signed identity keys.
    alice->upload_keys(request, [](const mtx::responses::UploadKeys &res, RequestErr err) {
        check_error(err);
        for (const auto &e : res.one_time_key_counts)
            EXPECT_EQ(e.second, 0);
    });

    alice->close();
}

TEST(Encryption, UploadOneTimeKeys)
{
    auto alice       = make_test_client();
    auto olm_account = std::make_shared<mtx::crypto::OlmClient>();
    olm_account->create_new_account();

    alice->login(
      "alice", "secret", [](const mtx::responses::Login &, RequestErr err) { check_error(err); });

    while (alice->access_token().empty())
        sleep();

    olm_account->set_user_id(alice->user_id().to_string());
    olm_account->set_device_id(alice->device_id());

    auto nkeys = olm_account->generate_one_time_keys(5, true);
    EXPECT_EQ(nkeys, 5);

    mtx::requests::UploadKeys req = olm_account->create_upload_keys_request();

    alice->upload_keys(req, [](const mtx::responses::UploadKeys &res, RequestErr err) {
        check_error(err);
        ASSERT_TRUE(res.one_time_key_counts.find("signed_curve25519") !=
                    res.one_time_key_counts.end());
        EXPECT_EQ(res.one_time_key_counts.at("signed_curve25519"), 5);
    });

    alice->close();
}

TEST(Encryption, UploadSignedOneTimeKeys)
{
    auto alice       = make_test_client();
    auto olm_account = std::make_shared<mtx::crypto::OlmClient>();
    olm_account->create_new_account();

    alice->login(
      "alice", "secret", [](const mtx::responses::Login &, RequestErr err) { check_error(err); });

    while (alice->access_token().empty())
        sleep();

    olm_account->set_user_id(alice->user_id().to_string());
    olm_account->set_device_id(alice->device_id());

    auto nkeys = olm_account->generate_one_time_keys(5);
    EXPECT_EQ(nkeys, 5);

    auto one_time_keys = olm_account->one_time_keys();

    mtx::requests::UploadKeys req;
    for (const auto &[key_id, key] : olm_account->sign_one_time_keys(one_time_keys))
        req.one_time_keys[key_id] = key;

    alice->upload_keys(req, [nkeys](const mtx::responses::UploadKeys &res, RequestErr err) {
        check_error(err);
        EXPECT_EQ(res.one_time_key_counts.size(), 1);
        EXPECT_EQ(res.one_time_key_counts.at("signed_curve25519"), nkeys);
    });

    alice->close();
}

TEST(Encryption, UploadKeys)
{
    auto alice       = make_test_client();
    auto olm_account = std::make_shared<mtx::crypto::OlmClient>();
    olm_account->create_new_account();

    alice->login(
      "alice", "secret", [](const mtx::responses::Login &, RequestErr err) { check_error(err); });

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
    auto alice     = make_test_client();
    auto alice_olm = std::make_shared<mtx::crypto::OlmClient>();

    auto bob     = make_test_client();
    auto bob_olm = std::make_shared<mtx::crypto::OlmClient>();

    alice_olm->create_new_account();
    bob_olm->create_new_account();

    alice->login(
      "alice", "secret", [](const mtx::responses::Login &, RequestErr err) { check_error(err); });

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

    bob->upload_keys(bob_req, [&uploads](const mtx::responses::UploadKeys &res, RequestErr err) {
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
      alice_rk, [&responses, bob, bob_req](const mtx::responses::QueryKeys &res, RequestErr err) {
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
      bob_rk, [&responses, alice, alice_req](const mtx::responses::QueryKeys &res, RequestErr err) {
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

    auto alice     = make_test_client();
    auto alice_olm = std::make_shared<OlmClient>();

    alice_olm->create_new_account();
    alice->login("alice", "secret", check_login);

    auto bob     = make_test_client();
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
          ASSERT_TRUE(verify_identity_signature(json(current_device).get<mtx::crypto::DeviceKeys>(),
                                                DeviceId(bob->device_id()),
                                                UserId(bob->user_id().to_string())));

          mtx::requests::ClaimKeys claim_keys;
          for (const auto &d : devices)
              claim_keys.one_time_keys[bob->user_id().to_string()][d] = SIGNED_CURVE25519;
          alice->claim_keys(
            claim_keys,
            [alice_olm, bob, bob_ed25519](const mtx::responses::ClaimKeys &res, RequestErr err) {
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
            });
      });

    alice->close();
    bob->close();
}

TEST(Encryption, ClaimMultipleDeviceKeys)
{
    using namespace mtx::crypto;

    // Login with alice multiple times
    auto alice1 = make_test_client();
    auto alice2 = make_test_client();
    auto alice3 = make_test_client();
    alice1->login("alice", "secret", check_login);
    alice2->login("alice", "secret", check_login);
    alice3->login("alice", "secret", check_login);

    WAIT_UNTIL(!alice1->access_token().empty() && !alice2->access_token().empty() &&
               !alice3->access_token().empty())

    atomic_int uploads(0);
    auto upload_cb = [&uploads](const mtx::responses::UploadKeys &res, RequestErr err) {
        check_error(err);
        EXPECT_EQ(res.one_time_key_counts.size(), 1);
        EXPECT_EQ(res.one_time_key_counts.at("signed_curve25519"), 10);
        uploads += 1;
    };

    auto olm1 = std::make_shared<OlmClient>();
    olm1->create_new_account();
    olm1->generate_one_time_keys(10);
    olm1->set_user_id(alice1->user_id().to_string());
    olm1->set_device_id(alice1->device_id());

    auto olm2 = std::make_shared<OlmClient>();
    olm2->create_new_account();
    olm2->generate_one_time_keys(10);
    olm2->set_user_id(alice2->user_id().to_string());
    olm2->set_device_id(alice2->device_id());

    auto olm3 = std::make_shared<OlmClient>();
    olm3->create_new_account();
    olm3->generate_one_time_keys(10);
    olm3->set_user_id(alice3->user_id().to_string());
    olm3->set_device_id(alice3->device_id());

    alice1->upload_keys(olm1->create_upload_keys_request(), upload_cb);
    alice2->upload_keys(olm2->create_upload_keys_request(), upload_cb);
    alice3->upload_keys(olm3->create_upload_keys_request(), upload_cb);

    WAIT_UNTIL(uploads == 3);

    // Bob will claim all keys from alice
    auto bob = make_test_client();
    bob->login("bob", "secret", check_login);

    WAIT_UNTIL(!bob->access_token().empty())

    std::vector<std::string> devices_;
    devices_.push_back(alice1->device_id());
    devices_.push_back(alice2->device_id());
    devices_.push_back(alice3->device_id());

    mtx::requests::ClaimKeys claim_keys;
    for (const auto &d : devices_)
        claim_keys.one_time_keys[alice1->user_id().to_string()][d] = SIGNED_CURVE25519;

    bob->claim_keys(claim_keys,
                    [user_id = alice1->user_id().to_string()](const mtx::responses::ClaimKeys &res,
                                                              RequestErr err) {
                        check_error(err);
                        auto retrieved_devices = res.one_time_keys.at(user_id);
                        EXPECT_EQ(retrieved_devices.size(), 3);
                    });

    bob->close();

    alice1->close();
    alice2->close();
    alice3->close();
}

TEST(Encryption, UploadCrossSigningKeys)
{
    auto alice       = make_test_client();
    auto olm_account = std::make_shared<mtx::crypto::OlmClient>();

    EXPECT_THROW(olm_account->identity_keys(), olm_exception);

    olm_account->create_new_account();

    alice->login(
      "alice", "secret", [](const mtx::responses::Login &, RequestErr err) { check_error(err); });

    while (alice->access_token().empty())
        sleep();

    olm_account->set_user_id(alice->user_id().to_string());
    olm_account->set_device_id(alice->device_id());

    auto id_keys = olm_account->identity_keys();

    ASSERT_TRUE(id_keys.curve25519.size() > 10);
    ASSERT_TRUE(id_keys.curve25519.size() > 10);

    mtx::crypto::OneTimeKeys unused;
    auto request = olm_account->create_upload_keys_request(unused, {});

    // Make the request with the signed identity keys.
    alice->upload_keys(
      request, [](const mtx::responses::UploadKeys &, RequestErr err) { check_error(err); });

    auto xsign_keys = olm_account->create_crosssigning_keys();
    ASSERT_TRUE(xsign_keys.has_value());
    mtx::requests::DeviceSigningUpload u;
    u.master_key       = xsign_keys->master_key;
    u.user_signing_key = xsign_keys->user_signing_key;
    u.self_signing_key = xsign_keys->self_signing_key;
    alice->device_signing_upload(
      u,
      mtx::http::UIAHandler([](const mtx::http::UIAHandler &h,
                               const mtx::user_interactive::Unauthorized &unauthorized) {
          ASSERT_EQ(unauthorized.flows.size(), 1);
          ASSERT_EQ(unauthorized.flows[0].stages.size(), 1);
          ASSERT_EQ(unauthorized.flows[0].stages[0], mtx::user_interactive::auth_types::password);

          mtx::user_interactive::Auth auth;
          auth.session = unauthorized.session;
          mtx::user_interactive::auth::Password pass{};
          pass.password        = "secret";
          pass.identifier_user = "alice";
          pass.identifier_type = mtx::user_interactive::auth::Password::IdType::UserId;
          auth.content         = pass;
          h.next(auth);
      }),
      [](RequestErr e) { check_error(e); });

    alice->close();
}

TEST(Encryption, UploadOnlineBackup)
{
    auto alice       = make_test_client();
    auto olm_account = std::make_shared<mtx::crypto::OlmClient>();

    EXPECT_THROW(olm_account->identity_keys(), olm_exception);

    olm_account->create_new_account();

    alice->login(
      "alice", "secret", [](const mtx::responses::Login &, RequestErr err) { check_error(err); });

    while (alice->access_token().empty())
        sleep();

    olm_account->set_user_id(alice->user_id().to_string());
    olm_account->set_device_id(alice->device_id());

    auto id_keys = olm_account->identity_keys();

    ASSERT_TRUE(id_keys.curve25519.size() > 10);
    ASSERT_TRUE(id_keys.curve25519.size() > 10);

    mtx::crypto::OneTimeKeys unused;
    auto request = olm_account->create_upload_keys_request(unused, {});

    // Make the request with the signed identity keys.
    alice->upload_keys(
      request, [](const mtx::responses::UploadKeys &, RequestErr err) { check_error(err); });

    auto xsign_keys = olm_account->create_crosssigning_keys();
    ASSERT_TRUE(xsign_keys.has_value());
    mtx::requests::DeviceSigningUpload u;
    u.master_key       = xsign_keys->master_key;
    u.user_signing_key = xsign_keys->user_signing_key;
    u.self_signing_key = xsign_keys->self_signing_key;
    alice->device_signing_upload(
      u,
      mtx::http::UIAHandler([](const mtx::http::UIAHandler &h,
                               const mtx::user_interactive::Unauthorized &unauthorized) {
          ASSERT_EQ(unauthorized.flows.size(), 1);
          ASSERT_EQ(unauthorized.flows[0].stages.size(), 1);
          ASSERT_EQ(unauthorized.flows[0].stages[0], mtx::user_interactive::auth_types::password);

          mtx::user_interactive::Auth auth;
          auth.session = unauthorized.session;
          mtx::user_interactive::auth::Password pass{};
          pass.password        = "secret";
          pass.identifier_user = "alice";
          pass.identifier_type = mtx::user_interactive::auth::Password::IdType::UserId;
          auth.content         = pass;
          h.next(auth);
      }),
      [xsign_keys, olm_account, alice](RequestErr e) {
          check_error(e);

          auto bk = olm_account->create_online_key_backup(xsign_keys->private_master_key);
          alice->post_backup_version(bk->backupVersion.algorithm,
                                     bk->backupVersion.auth_data,
                                     [](const mtx::responses::Version &v, RequestErr e) {
                                         check_error(e);

                                         EXPECT_FALSE(v.version.empty());
                                     });
      });

    alice->close();
}

TEST(Encryption, KeyChanges)
{
    auto carl     = make_test_client();
    auto carl_olm = std::make_shared<mtx::crypto::OlmClient>();
    carl_olm->create_new_account();

    carl->login(
      "carl", "secret", [](const mtx::responses::Login &, RequestErr err) { check_error(err); });

    while (carl->access_token().empty())
        sleep();

    carl_olm->set_device_id(carl->device_id());
    carl_olm->set_user_id(carl->user_id().to_string());

    mtx::requests::CreateRoom req;
    carl->create_room(req, [carl, carl_olm](const mtx::responses::CreateRoom &, RequestErr err) {
        check_error(err);

        // Carl syncs to get the first next_batch token.
        SyncOpts opts;
        opts.timeout = 0;
        carl->sync(opts, [carl, carl_olm](const mtx::responses::Sync &res, RequestErr err) {
            check_error(err);
            const auto next_batch_token = res.next_batch;

            auto key_req = ::generate_keys(carl_olm);

            // Changes his keys.
            carl->upload_keys(
              key_req,
              [carl, next_batch_token](const mtx::responses::UploadKeys &, RequestErr err) {
                  check_error(err);

                  // The key changes should contain his username
                  // because of the key uploading.
                  carl->key_changes(next_batch_token,
                                    "",
                                    [carl](const mtx::responses::KeyChanges &res, RequestErr err) {
                                        check_error(err);

                                        EXPECT_EQ(res.changed.size(), 1);
                                        EXPECT_EQ(res.left.size(), 0);

                                        EXPECT_EQ(res.changed.at(0), carl->user_id().to_string());
                                    });
              });
        });
    });

    carl->close();
}

TEST(Encryption, EnableEncryption)
{
    auto bob  = make_test_client();
    auto carl = make_test_client();

    bob->login("bob", "secret", [](const Login &, RequestErr err) { check_error(err); });
    carl->login("carl", "secret", [](const Login &, RequestErr err) { check_error(err); });

    while (bob->access_token().empty() || carl->access_token().empty())
        sleep();

    atomic_int responses(0);
    mtx::identifiers::Room joined_room;

    mtx::requests::CreateRoom req;
    req.invite = {"@carl:" + server_name()};
    bob->create_room(
      req,
      [bob, carl, &responses, &joined_room](const mtx::responses::CreateRoom &res, RequestErr err) {
          check_error(err);
          joined_room = res.room_id;

          bob->enable_encryption(res.room_id.to_string(),
                                 [&responses](const mtx::responses::EventId &, RequestErr err) {
                                     check_error(err);
                                     responses += 1;
                                 });

          carl->join_room(res.room_id.to_string(),
                          [&responses](const mtx::responses::RoomId &, RequestErr err) {
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
            if (std::holds_alternative<StateEvent<state::Encryption>>(e))
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

    auto session_id = mtx::crypto::session_id(outbound_session.get());
    ASSERT_FALSE(session_id.empty());
    auto session_key = mtx::crypto::session_key(outbound_session.get());
    ASSERT_FALSE(session_key.empty());
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
    ASSERT_EQ(1,
              matches_inbound_session_from(bob_inbound_session.get(), alice_key, ciphertext_str));

    // Bob validates that the message wasn't sent by someone else.
    ASSERT_EQ(0, matches_inbound_session_from(bob_inbound_session.get(), bob_key, ciphertext_str));

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
    auto alice_http = make_test_client();
    alice_olm->create_new_account();
    alice_olm->generate_one_time_keys(10);

    auto bob_olm  = std::make_shared<OlmClient>();
    auto bob_http = make_test_client();
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

                               bob_ed25519 = devices.at(device_id).keys.at("ed25519:" + device_id);
                               bob_curve25519 =
                                 devices.at(device_id).keys.at("curve25519:" + device_id);

                               request_finished = true;
                           });

    WAIT_UNTIL(request_finished);

    // Alice needs one of Bob's one time keys.
    request_finished = false;

    mtx::requests::ClaimKeys claim_keys;
    claim_keys.one_time_keys[bob_http->user_id().to_string()][bob_http->device_id()] =
      SIGNED_CURVE25519;

    alice_http->claim_keys(claim_keys,
                           [&bob_otk, bob = bob_http, &request_finished](
                             const mtx::responses::ClaimKeys &res, RequestErr err) {
                               check_error(err);

                               const auto user_id   = bob->user_id().to_string();
                               const auto device_id = bob->device_id();

                               auto retrieved_devices = res.one_time_keys.at(user_id);
                               for (const auto &device : retrieved_devices) {
                                   if (device.first == device_id) {
                                       bob_otk =
                                         device.second.begin()->at("key").get<std::string>();
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
    json payload =
      json{{"content", {{"secret", SECRET_TEXT}}}, {"type", "im.nheko.custom_test_event"}};

    // Alice creates an outbound session.
    auto out_session = alice_olm->create_outbound_session(bob_curve25519, bob_otk);
    auto device_msg  = alice_olm->create_olm_encrypted_content(out_session.get(),
                                                              payload,
                                                              UserId("@bob:" + server_name()),
                                                              bob_olm->identity_keys().ed25519,
                                                              bob_curve25519);

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

          EXPECT_EQ(res.to_device.events.size(), 1);

          auto olm_msg =
            std::get<DeviceEvent<mtx::events::msg::OlmEncrypted>>(res.to_device.events[0]).content;
          auto cipher = olm_msg.ciphertext.begin();

          EXPECT_EQ(cipher->first, bob->identity_keys().curve25519);

          const auto msg_body = cipher->second.body;
          const auto msg_type = cipher->second.type;

          assert(msg_type == 0);

          auto inbound_session = bob->create_inbound_session(msg_body);
          ASSERT_TRUE(
            matches_inbound_session_from(inbound_session.get(), olm_msg.sender_key, msg_body));

          auto output = bob->decrypt_message(inbound_session.get(), msg_type, msg_body);

          // Parsing the original plaintext json object.
          auto plaintext     = json::parse(std::string((char *)output.data(), output.size()));
          std::string secret = plaintext.at("content").at("secret").get<std::string>();

          ASSERT_EQ(secret, SECRET_TEXT);
      });

    alice_http->close();
    bob_http->close();
}

TEST(Encryption, ShareSecret)
{
    // Alice wants to use olm to send data to Bob.
    auto alice_olm  = std::make_shared<OlmClient>();
    auto alice_http = make_test_client();
    alice_olm->create_new_account();
    alice_olm->generate_one_time_keys(10);

    auto bob_olm  = std::make_shared<OlmClient>();
    auto bob_http = make_test_client();
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

                               bob_ed25519 = devices.at(device_id).keys.at("ed25519:" + device_id);
                               bob_curve25519 =
                                 devices.at(device_id).keys.at("curve25519:" + device_id);

                               request_finished = true;
                           });

    WAIT_UNTIL(request_finished);

    // Alice needs one of Bob's one time keys.
    request_finished = false;

    mtx::requests::ClaimKeys claim_keys;
    claim_keys.one_time_keys[bob_http->user_id().to_string()][bob_http->device_id()] =
      SIGNED_CURVE25519;

    alice_http->claim_keys(claim_keys,
                           [&bob_otk, bob = bob_http, &request_finished](
                             const mtx::responses::ClaimKeys &res, RequestErr err) {
                               check_error(err);

                               const auto user_id   = bob->user_id().to_string();
                               const auto device_id = bob->device_id();

                               auto retrieved_devices = res.one_time_keys.at(user_id);
                               for (const auto &device : retrieved_devices) {
                                   if (device.first == device_id) {
                                       bob_otk =
                                         device.second.begin()->at("key").get<std::string>();
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
    constexpr auto REQUEST_ID  = "askjfdgsdfgfdg";

    // Alice create m.room.key request
    mtx::events::msg::SecretRequest secretRequest{};
    secretRequest.request_id           = REQUEST_ID;
    secretRequest.name                 = "abcdefg";
    secretRequest.requesting_device_id = bob_http->device_id();
    secretRequest.action               = RequestAction::Request;

    SyncOpts opts;
    opts.timeout = 0;

    // Finally sends the olm encrypted message to Bob's device.
    atomic_bool is_sent(false);
    bob_http->send_to_device<SecretRequest>(
      bob_http->generate_txn_id(),
      {{alice_http->user_id(), {{alice_http->device_id(), secretRequest}}}},
      [&](RequestErr err) {
          check_error(err);

          alice_http->sync(opts, [&](const mtx::responses::Sync &res, RequestErr err) {
              check_error(err);

              mtx::events::DeviceEvent<mtx::events::msg::SecretSend> secretSend{};
              secretSend.content.secret = SECRET_TEXT;
              secretSend.content.request_id =
                std::get<mtx::events::DeviceEvent<mtx::events::msg::SecretRequest>>(
                  res.to_device.events.at(0))
                  .content.request_id;
              secretSend.type = mtx::events::EventType::SecretSend;

              json payload = secretSend;

              // Alice creates an outbound session.
              auto out_session = alice_olm->create_outbound_session(bob_curve25519, bob_otk);
              auto device_msg =
                alice_olm->create_olm_encrypted_content(out_session.get(),
                                                        payload,
                                                        UserId("@bob:" + server_name()),
                                                        bob_olm->identity_keys().ed25519,
                                                        bob_curve25519);

              std::map<mtx::identifiers::User,
                       std::map<std::string, mtx::events::msg::OlmEncrypted>>
                body{{bob_http->user_id(),
                      {{bob_http->device_id(), device_msg.get<mtx::events::msg::OlmEncrypted>()}}}};
              alice_http->send_to_device<OlmEncrypted>(
                alice_http->generate_txn_id(), body, [&is_sent](RequestErr err) {
                    check_error(err);
                    is_sent = true;
                });
          });
      });

    WAIT_UNTIL(is_sent)

    bob_http->sync(
      opts,
      [bob = bob_olm, SECRET_TEXT, REQUEST_ID](const mtx::responses::Sync &res, RequestErr err) {
          check_error(err);

          EXPECT_EQ(res.to_device.events.size(), 1);

          auto olm_msg =
            std::get<DeviceEvent<mtx::events::msg::OlmEncrypted>>(res.to_device.events[0]).content;
          auto cipher = olm_msg.ciphertext.begin();

          EXPECT_EQ(cipher->first, bob->identity_keys().curve25519);

          const auto msg_body = cipher->second.body;
          const auto msg_type = cipher->second.type;

          assert(msg_type == 0);

          auto inbound_session = bob->create_inbound_session(msg_body);
          ASSERT_TRUE(
            matches_inbound_session_from(inbound_session.get(), olm_msg.sender_key, msg_body));

          auto output = bob->decrypt_message(inbound_session.get(), msg_type, msg_body);

          // Parsing the original plaintext json object.
          auto ev = json::parse(std::string_view((char *)output.data(), output.size()))
                      .get<mtx::events::DeviceEvent<mtx::events::msg::SecretSend>>();

          ASSERT_EQ(ev.content.secret, SECRET_TEXT);
          ASSERT_EQ(ev.content.request_id, REQUEST_ID);
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

    auto outbound_session   = alice->create_outbound_session(bob_key, bob_one_time_key);
    auto initial_session_id = session_id(outbound_session.get());

    auto plaintext      = "Hello, Bob!";
    size_t msgtype      = olm_encrypt_message_type(outbound_session.get());
    auto ciphertext     = alice->encrypt_message(outbound_session.get(), plaintext);
    auto ciphertext_str = std::string((char *)ciphertext.data(), ciphertext.size());

    EXPECT_EQ(msgtype, 0);

    auto saved_outbound_session    = pickle<SessionObject>(outbound_session.get(), "wat");
    auto restored_outbound_session = unpickle<SessionObject>(saved_outbound_session, "wat");

    EXPECT_EQ(session_id(restored_outbound_session.get()), initial_session_id);

    EXPECT_THROW(unpickle<SessionObject>(saved_outbound_session, "another_secret"), olm_exception);

    msgtype = olm_encrypt_message_type(restored_outbound_session.get());
    EXPECT_EQ(msgtype, 0);

    auto restored_ciphertext = alice->encrypt_message(restored_outbound_session.get(), plaintext);
    auto restored_ciphertext_str =
      std::string((char *)restored_ciphertext.data(), restored_ciphertext.size());

    auto inbound_session          = bob->create_inbound_session(ciphertext_str);
    auto saved_inbound_session    = pickle<SessionObject>(inbound_session.get(), "woot");
    auto restored_inbound_session = unpickle<SessionObject>(saved_inbound_session, "woot");

    EXPECT_THROW(unpickle<SessionObject>(saved_inbound_session, "another_secret"), olm_exception);

    ASSERT_EQ(1, matches_inbound_session(inbound_session.get(), ciphertext_str));
    ASSERT_EQ(1, matches_inbound_session(inbound_session.get(), restored_ciphertext_str));
    ASSERT_EQ(1, matches_inbound_session(restored_inbound_session.get(), restored_ciphertext_str));
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
    EXPECT_THROW(unpickle<OutboundSessionObject>(saved_session, "another_secret"), olm_exception);

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

    EXPECT_EQ(std::string((char *)plaintext.data.data(), plaintext.data.size()),
              std::string((char *)restored_plaintext.data.data(), restored_plaintext.data.size()));

    EXPECT_EQ(std::string((char *)plaintext.data.data(), plaintext.data.size()), SECRET);

    EXPECT_THROW(alice->decrypt_group_message(inbound_session.get(), ""), olm_exception);
    EXPECT_THROW(alice->decrypt_group_message(nullptr, ""), olm_exception);
    EXPECT_THROW(alice->decrypt_group_message(nullptr, ciphertext), olm_exception);
}

TEST(ExportSessions, InboundMegolmSessions)
{
    auto alice = std::make_shared<OlmClient>();
    alice->create_new_account();
    alice->generate_one_time_keys(1);

    auto bob = std::make_shared<OlmClient>();
    bob->create_new_account();
    bob->generate_one_time_keys(1);

    // ==================== SESSION SETUP =================== //

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

    // Bob will use the session_key to create an inbound megolm session.
    // The session_id will be used to map future messages to this session.
    auto inbound_megolm_session = bob->init_inbound_group_session(session_key);

    // Bob can finally decrypt Alice's original message.
    auto ciphertext =
      std::string((char *)encrypted_secret_message.data(), encrypted_secret_message.size());
    auto bob_plaintext = bob->decrypt_group_message(inbound_megolm_session.get(), ciphertext);

    auto output_str = std::string((char *)bob_plaintext.data.data(), bob_plaintext.data.size());
    ASSERT_EQ(output_str, secret_message);

    // ==================== SESSION IMPORT/EXPORT =================== //

    auto exported_session_key     = export_session(inbound_megolm_session.get(), -1);
    auto restored_inbound_session = import_session(exported_session_key);

    // Decrypt message again.
    auto restored_ciphertext =
      std::string((char *)encrypted_secret_message.data(), encrypted_secret_message.size());
    auto restored_plaintext =
      bob->decrypt_group_message(restored_inbound_session.get(), restored_ciphertext);

    auto restored_output_str =
      std::string((char *)restored_plaintext.data.data(), restored_plaintext.data.size());
    ASSERT_EQ(restored_output_str, secret_message);
}

TEST(Encryption, EncryptedFile)
{
    {
        auto buffer = mtx::crypto::create_buffer(16);
        ASSERT_EQ(buffer.size(), 16);
        auto buf_str = mtx::crypto::to_string(buffer);
        ASSERT_EQ(buf_str.size(), 16);
    }

    std::string plaintext = "This is some plain text payload";
    auto encryption_data  = mtx::crypto::encrypt_file(plaintext);
    ASSERT_NE(plaintext, mtx::crypto::to_string(encryption_data.first));
    ASSERT_EQ(plaintext,
              mtx::crypto::to_string(mtx::crypto::decrypt_file(
                mtx::crypto::to_string(encryption_data.first), encryption_data.second)));
    // key needs to be 32 bytes/256 bits
    ASSERT_EQ(32, mtx::crypto::base642bin_urlsafe_unpadded(encryption_data.second.key.k).size());
    // IV needs to be 16 bytes/128 bits
    ASSERT_EQ(16, mtx::crypto::base642bin_unpadded(encryption_data.second.iv).size());

    json j = R"({
  "type": "m.room.message",
  "content": {
    "body": "test.txt",
    "info": {
      "size": 8,
      "mimetype": "text/plain"
    },
    "msgtype": "m.file",
    "file": {
      "v": "v2",
      "key": {
        "alg": "A256CTR",
        "ext": true,
        "k": "6osKLzUKV1YZ06WEX0b77D784Te8oAj5eNU-gAgkjs4",
        "key_ops": [
          "encrypt",
          "decrypt"
        ],
        "kty": "oct"
      },
      "iv": "7zRP/t89YWcAAAAAAAAAAA",
      "hashes": {
        "sha256": "5g41hn7n10sCw3+2j7CQ9SJl6R/v5EBT4MshdFgHhzo"
      },
      "url": "mxc://neko.dev/WPKoOAPfPlcHiZZTEoaIoZhN",
      "mimetype": "text/plain"
    }
 },
 "event_id": "$1575320135447DEPky:neko.dev",
  "origin_server_ts": 1575320135324,
  "sender": "@test:neko.dev",
  "unsigned": {
    "age": 1081,
    "transaction_id": "m1575320142400.8"
  },
  "room_id": "!YnUlhwgbBaGcAFsJOJ:neko.dev"
})"_json;
    mtx::events::RoomEvent<mtx::events::msg::File> ev =
      j.get<mtx::events::RoomEvent<mtx::events::msg::File>>();

    ASSERT_EQ("abcdefg\n",
              mtx::crypto::to_string(
                mtx::crypto::decrypt_file("=\xFDX\xAB\xCA\xEB\x8F\xFF", ev.content.file.value())));
}

TEST(Encryption, SAS)
{
    auto alice = std::make_shared<OlmClient>();
    alice->create_new_account();
    auto bob = std::make_shared<OlmClient>();
    bob->create_new_account();

    auto alice_sas = alice->sas_init();
    auto bob_sas   = bob->sas_init();

    ASSERT_EQ(alice_sas->public_key().length(), 43);
    ASSERT_EQ(bob_sas->public_key().length(), 43);

    alice_sas->set_their_key(bob_sas->public_key());
    bob_sas->set_their_key(alice_sas->public_key());

    std::string info = "test_info";

    std::vector<int> alice_decimal = alice_sas->generate_bytes_decimal(info);
    std::vector<int> bob_decimal   = bob_sas->generate_bytes_decimal(info);

    ASSERT_EQ(alice_decimal.size(), 3);
    ASSERT_EQ(bob_decimal.size(), 3);

    for (int i = 0; i < 3; ++i) {
        ASSERT_TRUE((alice_decimal[i] >= 0) && (alice_decimal[i] <= 8191));
        ASSERT_TRUE((bob_decimal[i] >= 0) && (bob_decimal[i] <= 8191));
        ASSERT_EQ(alice_decimal[i], bob_decimal[i]);
    }

    std::vector<int> alice_emoji = alice_sas->generate_bytes_emoji(info);
    std::vector<int> bob_emoji   = bob_sas->generate_bytes_emoji(info);

    ASSERT_EQ(alice_emoji.size(), 7);
    ASSERT_EQ(bob_emoji.size(), 7);

    for (int i = 0; i < 7; ++i) {
        ASSERT_TRUE((alice_emoji[i] >= 0) && (alice_emoji[i] <= 8191));
        ASSERT_TRUE((bob_emoji[i] >= 0) && (bob_emoji[i] <= 8191));
        ASSERT_EQ(alice_emoji[i], bob_emoji[i]);
    }
}

TEST(Encryption, DISABLED_HandleRoomKeyEvent) {}
TEST(Encryption, DISABLED_HandleRoomKeyRequestEvent) {}
TEST(Encryption, DISABLED_HandleNewDevices) {}
TEST(Encryption, DISABLED_HandleLeftDevices) {}

TEST(Encryption, DISABLED_SendEncryptedMessageWithMegolm) {}
TEST(Encryption, DISABLED_RotateMegolmSession) {}
