#include <gtest/gtest.h>

#include "crypto.hpp"
#include "json.hpp"

#include <olm/olm.h>
#include <olm/utility.hh>

using json = nlohmann::json;

using namespace mtx::client::crypto;
using namespace std;

TEST(Utilities, CanonicalJSON)
{
        // Examples taken from
        // https://matrix.org/docs/spec/appendices.html#canonical-json
        json data = R"({
        "auth": {
          "success": true,
          "mxid": "@john.doe:example.com",
          "profile": {
            "display_name": "John Doe",
            "three_pids": [{
              "medium": "email",
              "address": "john.doe@example.org"
            }, {
              "medium": "msisdn",
              "address": "123456789"
            }]
          }}})"_json;

        EXPECT_EQ(
          data.dump(),
          "{\"auth\":{\"mxid\":\"@john.doe:example.com\",\"profile\":{\"display_name\":\"John "
          "Doe\",\"three_pids\":[{\"address\":\"john.doe@example.org\",\"medium\":\"email\"},{"
          "\"address\":\"123456789\",\"medium\":\"msisdn\"}]},\"success\":true}}");

        json data0 = R"({"b":"2","a":"1"})"_json;
        EXPECT_EQ(data0.dump(), "{\"a\":\"1\",\"b\":\"2\"}");

        json data1 = R"({ "本": 2, "日": 1 })"_json;
        EXPECT_EQ(data1.dump(), "{\"日\":1,\"本\":2}");

        json data2 = R"({"a": "\u65E5"})"_json;
        EXPECT_EQ(data2.dump(), "{\"a\":\"日\"}");

        json data3 = R"({ "a": null })"_json;
        EXPECT_EQ(data3.dump(), "{\"a\":null}");
}

TEST(Utilities, JsonToBuffer)
{
        auto msg = json({{"key", "text"}});
        auto buf = to_buffer(msg);

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

        auto sig_buf = to_buffer(alice->sign_message(msg));

        auto res = olm_ed25519_verify(alice->utility(),
                                      to_buffer(alice->identity_keys().ed25519)->data(),
                                      to_buffer(alice->identity_keys().ed25519)->size(),
                                      to_buffer(msg)->data(),
                                      to_buffer(msg)->size(),
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

        auto sig_buf = to_buffer(alice->sign_message(msg));

        auto res = olm_ed25519_verify(alice->utility(),
                                      to_buffer(alice->identity_keys().ed25519)->data(),
                                      to_buffer(alice->identity_keys().ed25519)->size(),
                                      to_buffer(msg)->data(),
                                      to_buffer(msg)->size(),
                                      sig_buf->data(),
                                      sig_buf->size());

        EXPECT_EQ(std::string(olm_utility_last_error(alice->utility())), "SUCCESS");
        EXPECT_EQ(res, 0);
}
