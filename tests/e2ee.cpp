#include <atomic>
#include <chrono>
#include <iostream>
#include <thread>

#include <boost/algorithm/string.hpp>

#include <gtest/gtest.h>

#include "olm/account.hh"

#include "client.hpp"
#include "crypto.hpp"

#include "mtx/requests.hpp"
#include "mtx/responses.hpp"
#include "variant.hpp"

using namespace mtx::client;
using namespace mtx::identifiers;
using namespace mtx::events::collections;

using namespace std;

using ErrType = std::experimental::optional<errors::ClientError>;

std::map<std::string, json>
sign_one_time_keys(std::shared_ptr<olm::Account> account,
                   const mtx::client::crypto::OneTimeKeys &keys,
                   const mtx::identifiers::User &user_id,
                   const std::string &device_id)
{
        // Sign & append the one time keys.
        std::map<std::string, json> signed_one_time_keys;
        for (const auto &elem : keys.curve25519) {
                auto sig = mtx::client::crypto::sign_one_time_key(account, elem.second);

                signed_one_time_keys["signed_curve25519:" + elem.first] =
                  mtx::client::crypto::signed_one_time_key_json(
                    user_id, device_id, elem.second, sig);
        }

        return signed_one_time_keys;
}

mtx::requests::UploadKeys
create_upload_keys_request(std::shared_ptr<olm::Account> account,
                           const mtx::client::crypto::IdentityKeys &identity_keys,
                           const mtx::client::crypto::OneTimeKeys &one_time_keys,
                           const mtx::identifiers::User &user_id,
                           const string &device_id)
{
        mtx::requests::UploadKeys req;
        req.device_keys.user_id   = user_id.to_string();
        req.device_keys.device_id = device_id;

        req.device_keys.keys["curve25519:" + device_id] = identity_keys.curve25519;
        req.device_keys.keys["ed25519:" + device_id]    = identity_keys.ed25519;

        // Generate and add the signature to the request.
        auto sig = sign_identity_keys(account, identity_keys, user_id, device_id);
        req.device_keys.signatures[user_id.to_string()]["ed25519:" + device_id] = sig;

        if (one_time_keys.curve25519.empty())
                return req;

        // Sign & append the one time keys.
        req.one_time_keys = ::sign_one_time_keys(account, one_time_keys, user_id, device_id);

        return req;
}

void
check_error(ErrType err)
{
        if (err) {
                cout << "matrix (error)  : " << err->matrix_error.error << "\n";
                cout << "matrix (errcode): " << mtx::errors::to_string(err->matrix_error.errcode)
                     << "\n";
                cout << "error_code      : " << err->error_code << "\n";
                cout << "status_code     : " << err->status_code << "\n";

                if (!err->parse_error.empty())
                        cout << "parse_error     : " << err->parse_error << "\n";
        }

        ASSERT_FALSE(err);
}

TEST(Encryption, UploadIdentityKeys)
{
        auto alice       = std::make_shared<Client>("localhost");
        auto olm_account = mtx::client::crypto::olm_new_account();

        alice->login(
          "alice", "secret", [](const mtx::responses::Login &, ErrType err) { check_error(err); });

        while (alice->access_token().empty())
                std::this_thread::sleep_for(std::chrono::milliseconds(100));

        auto identity_keys = mtx::client::crypto::identity_keys(olm_account);

        ASSERT_TRUE(identity_keys.curve25519.size() > 10);
        ASSERT_TRUE(identity_keys.curve25519.size() > 10);

        mtx::client::crypto::OneTimeKeys unused;
        auto request = ::create_upload_keys_request(
          olm_account, identity_keys, unused, alice->user_id(), alice->device_id());

        // Make the request with the signed identity keys.
        alice->upload_keys(request, [](const mtx::responses::UploadKeys &res, ErrType err) {
                check_error(err);
                EXPECT_EQ(res.one_time_key_counts.size(), 0);
        });

        alice->close();
}

TEST(Encryption, UploadOneTimeKeys)
{
        auto alice       = std::make_shared<Client>("localhost");
        auto olm_account = mtx::client::crypto::olm_new_account();

        alice->login(
          "alice", "secret", [](const mtx::responses::Login &, ErrType err) { check_error(err); });

        while (alice->access_token().empty())
                std::this_thread::sleep_for(std::chrono::milliseconds(100));

        auto nkeys = mtx::client::crypto::generate_one_time_keys(olm_account, 5);
        EXPECT_EQ(nkeys, 5);

        auto one_time_keys = mtx::client::crypto::one_time_keys(olm_account);

        mtx::requests::UploadKeys req;

        // Create the proper structure for uploading.
        std::map<std::string, json> unsigned_keys;

        auto obj = one_time_keys.at("curve25519");
        for (auto it = obj.begin(); it != obj.end(); ++it)
                unsigned_keys["curve25519:" + it.key()] = it.value();

        req.one_time_keys = unsigned_keys;

        alice->upload_keys(req, [](const mtx::responses::UploadKeys &res, ErrType err) {
                check_error(err);
                EXPECT_EQ(res.one_time_key_counts.size(), 1);
                EXPECT_EQ(res.one_time_key_counts.at("curve25519"), 5);
        });

        alice->close();
}

TEST(Encryption, UploadSignedOneTimeKeys)
{
        auto alice       = std::make_shared<Client>("localhost");
        auto olm_account = mtx::client::crypto::olm_new_account();

        alice->login(
          "alice", "secret", [](const mtx::responses::Login &, ErrType err) { check_error(err); });

        while (alice->access_token().empty())
                std::this_thread::sleep_for(std::chrono::milliseconds(100));

        auto nkeys = mtx::client::crypto::generate_one_time_keys(olm_account, 5);
        EXPECT_EQ(nkeys, 5);

        auto one_time_keys = mtx::client::crypto::one_time_keys(olm_account);

        mtx::requests::UploadKeys req;
        req.one_time_keys =
          ::sign_one_time_keys(olm_account, one_time_keys, alice->user_id(), alice->device_id());

        alice->upload_keys(req, [nkeys](const mtx::responses::UploadKeys &res, ErrType err) {
                check_error(err);
                EXPECT_EQ(res.one_time_key_counts.size(), 1);
                EXPECT_EQ(res.one_time_key_counts.at("signed_curve25519"), nkeys);
        });

        alice->close();
}

TEST(Encryption, UploadKeys)
{
        auto alice       = std::make_shared<Client>("localhost");
        auto olm_account = mtx::client::crypto::olm_new_account();

        alice->login(
          "alice", "secret", [](const mtx::responses::Login &, ErrType err) { check_error(err); });

        while (alice->access_token().empty())
                std::this_thread::sleep_for(std::chrono::milliseconds(100));

        json identity_keys = mtx::client::crypto::identity_keys(olm_account);

        mtx::client::crypto::generate_one_time_keys(olm_account, 1);
        auto one_time_keys = mtx::client::crypto::one_time_keys(olm_account);

        auto req = ::create_upload_keys_request(
          olm_account, identity_keys, one_time_keys, alice->user_id(), alice->device_id());

        alice->upload_keys(req, [](const mtx::responses::UploadKeys &res, ErrType err) {
                check_error(err);
                EXPECT_EQ(res.one_time_key_counts.size(), 1);
                EXPECT_EQ(res.one_time_key_counts.at("signed_curve25519"), 1);
        });

        alice->close();
}
