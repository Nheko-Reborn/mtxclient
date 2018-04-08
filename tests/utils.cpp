#include <gtest/gtest.h>

#include "crypto.hpp"
#include "json.hpp"

#include <olm/olm.h>
#include <olm/utility.hh>

using json = nlohmann::json;

using namespace mtx::client::crypto;
using namespace std;

constexpr int SIGNATURE_SIZE = 64;

TEST(Utilities, JsonToBuffer)
{
        auto msg = json({{"key", "text"}});
        auto buf = json_to_buffer(msg);

        EXPECT_EQ(std::string(buf->begin(), buf->end()), msg.dump());
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
                                           str_to_buffer(msg)->data(),
                                           msg.size(),
                                           sig_buf->data(),
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
                                           str_to_buffer(msg)->data(),
                                           msg.size(),
                                           sig_buf->data(),
                                           SIGNATURE_SIZE);

        EXPECT_EQ(utillity.last_error, 0);
        EXPECT_EQ(res, 0);
}

TEST(Utilities, OutboundGroupSession)
{
        auto alice = olm_new_account();
        auto bob   = olm_new_account();
        auto carl  = olm_new_account();

        generate_one_time_keys(bob, 1);
        generate_one_time_keys(carl, 1);

        OneTimeKeys bob_otk = one_time_keys(bob);
        IdentityKeys bob_ik = identity_keys(bob);

        OneTimeKeys carl_otk = one_time_keys(carl);
        IdentityKeys carl_ik = identity_keys(carl);

        auto bob_session =
          init_outbound_group_session(alice, bob_ik.curve25519, bob_otk.curve25519.begin()->second);
        auto carl_session = init_outbound_group_session(
          alice, carl_ik.curve25519, carl_otk.curve25519.begin()->second);

        auto sid_1 = create_buffer(bob_session.session_id_length());
        bob_session.session_id(sid_1->data(), sid_1->size());

        EXPECT_EQ(sid_1->size(), 32);

        auto sid_2 = create_buffer(carl_session.session_id_length());
        carl_session.session_id(sid_2->data(), sid_2->size());

        EXPECT_EQ(sid_2->size(), 32);
}
