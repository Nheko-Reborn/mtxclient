#include <gtest/gtest.h>

#include "crypto.hpp"
#include "json.hpp"

#include "olm/utility.hh"

using json = nlohmann::json;
using namespace mtx::client::crypto;

constexpr int SIGNATURE_SIZE = 64;

TEST(Utilities, JsonToBuffer)
{
        auto msg = json({{"key", "text"}});
        auto buf = json_to_buffer(msg);

        auto strjson = msg.dump();
        auto len     = msg.dump().size();

        for (uint8_t i = 0; i < len; i++) {
                if (strjson[i] != buf[i])
                        FAIL();
        }
}

TEST(Utilities, VerifySignedOneTimeKey)
{
        auto alice = olm_new_account();

        generate_one_time_keys(alice, 1);
        auto keys = one_time_keys(alice);

        auto first_key = keys["curve25519"].begin()->get<std::string>();
        auto msg       = json({{"key", first_key}}).dump();

        auto sig_buf = sign_message(alice, msg);

        olm::Utility utillity;

        auto res = utillity.ed25519_verify(alice->identity_keys.ed25519_key.public_key,
                                           str_to_buffer(msg).get(),
                                           msg.size(),
                                           sig_buf.get(),
                                           SIGNATURE_SIZE);

        EXPECT_EQ(utillity.last_error, 0);
        EXPECT_EQ(res, 0);
}

TEST(Utilities, VerifySignedIdentityKeys)
{
        auto alice = olm_new_account();

        json keys = identity_keys(alice);

        auto msg = json({{"algorithms", {"m.olm.v1.curve25519-aes-sha2", "m.megolm.v1.aes-sha2"}},
                         {"device_id", "some_device"},
                         {"user_id", "@alice:localhost"},
                         {"keys",
                          {{"curve25519:some_device", keys["curve25519"]},
                           {"ed25519:some_device", keys["ed25519"]}}}})
                     .dump();

        auto sig_buf = sign_message(alice, msg);

        olm::Utility utillity;

        auto res = utillity.ed25519_verify(alice->identity_keys.ed25519_key.public_key,
                                           str_to_buffer(msg).get(),
                                           msg.size(),
                                           sig_buf.get(),
                                           SIGNATURE_SIZE);

        EXPECT_EQ(utillity.last_error, 0);
        EXPECT_EQ(res, 0);
}
