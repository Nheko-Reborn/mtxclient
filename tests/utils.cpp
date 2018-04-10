#include <gtest/gtest.h>

#include "crypto.hpp"
#include "json.hpp"

#include <olm/olm.h>
#include <olm/utility.hh>

using json = nlohmann::json;

using namespace mtx::client::crypto;
using namespace std;

TEST(Utilities, JsonToBuffer)
{
        auto msg = json({{"key", "text"}});
        auto buf = json_to_buffer(msg);

        EXPECT_EQ(std::string(buf->begin(), buf->end()), msg.dump());
}

TEST(Utilities, VerifySignedOneTimeKey)
{
        auto alice = make_shared<OlmClient>();
        alice->create_new_account();
        alice->create_new_utility();

        alice->identity_keys();

        alice->generate_one_time_keys(1);
        auto keys = alice->one_time_keys();

        auto first_key = keys.curve25519.begin()->second;
        auto msg       = json({{"key", first_key}}).dump();

        auto sig_buf = alice->sign_message(msg);

        auto res = olm_ed25519_verify(alice->utility(),
                                      str_to_buffer(alice->identity_keys().ed25519)->data(),
                                      str_to_buffer(alice->identity_keys().ed25519)->size(),
                                      str_to_buffer(msg)->data(),
                                      str_to_buffer(msg)->size(),
                                      sig_buf->data(),
                                      sig_buf->size());

        EXPECT_EQ(std::string(olm_utility_last_error(alice->utility())), "SUCCESS");
        EXPECT_EQ(res, 0);
}

TEST(Utilities, VerifySignedIdentityKeys)
{
        auto alice = make_shared<OlmClient>();
        alice->create_new_account();
        alice->create_new_utility();

        json keys = alice->identity_keys();

        auto msg = json({{"algorithms", {"m.olm.v1.curve25519-aes-sha2", "m.megolm.v1.aes-sha2"}},
                         {"device_id", "some_device"},
                         {"user_id", "@alice:localhost"},
                         {"keys",
                          {{"curve25519:some_device", keys["curve25519"]},
                           {"ed25519:some_device", keys["ed25519"]}}}})
                     .dump();

        auto sig_buf = alice->sign_message(msg);

        auto res = olm_ed25519_verify(alice->utility(),
                                      str_to_buffer(alice->identity_keys().ed25519)->data(),
                                      str_to_buffer(alice->identity_keys().ed25519)->size(),
                                      str_to_buffer(msg)->data(),
                                      str_to_buffer(msg)->size(),
                                      sig_buf->data(),
                                      sig_buf->size());

        EXPECT_EQ(std::string(olm_utility_last_error(alice->utility())), "SUCCESS");
        EXPECT_EQ(res, 0);
}
