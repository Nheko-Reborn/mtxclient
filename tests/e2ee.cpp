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

        json identity_keys = mtx::client::crypto::identity_keys(olm_account);

        ASSERT_TRUE(identity_keys.find("curve25519") != identity_keys.end());
        ASSERT_TRUE(identity_keys.at("curve25519").get<std::string>().size() > 10);

        ASSERT_TRUE(identity_keys.find("ed25519") != identity_keys.end());
        ASSERT_TRUE(identity_keys.at("ed25519").get<std::string>().size() > 10);

        alice->upload_identity_keys(identity_keys,
                                    [](const mtx::responses::UploadKeys &res, ErrType err) {
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

        // Create the proper structure for uploading.
        std::map<std::string, json> keys;

        auto obj = one_time_keys.at("curve25519");
        for (auto it = obj.begin(); it != obj.end(); ++it)
                keys["curve25519:" + it.key()] = it.value();

        alice->upload_one_time_keys(keys, [](const mtx::responses::UploadKeys &res, ErrType err) {
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

        // Create the proper structure for uploading.
        std::map<std::string, json> signed_keys;

        auto obj = one_time_keys.at("curve25519");
        for (auto it = obj.begin(); it != obj.end(); ++it) {
                auto sig = mtx::client::crypto::sign_one_time_key(olm_account, it.value());

                signed_keys["signed_curve25519:" + it.key()] =
                  mtx::client::crypto::signed_one_time_key_json(
                    alice->user_id(), alice->device_id(), it.value(), sig);
        }

        alice->upload_one_time_keys(
          signed_keys, [nkeys](const mtx::responses::UploadKeys &res, ErrType err) {
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

        // Create the proper structure for uploading.
        std::map<std::string, json> signed_keys;

        auto obj = one_time_keys.at("curve25519");
        for (auto it = obj.begin(); it != obj.end(); ++it) {
                auto sig = mtx::client::crypto::sign_one_time_key(olm_account, it.value());

                signed_keys["signed_curve25519:" + it.key()] =
                  mtx::client::crypto::signed_one_time_key_json(
                    alice->user_id(), alice->device_id(), it.value(), sig);
        }

        alice->upload_keys(
          identity_keys, signed_keys, [](const mtx::responses::UploadKeys &res, ErrType err) {
                  check_error(err);
                  EXPECT_EQ(res.one_time_key_counts.size(), 1);
                  EXPECT_EQ(res.one_time_key_counts.at("signed_curve25519"), 1);
          });

        alice->close();
}
