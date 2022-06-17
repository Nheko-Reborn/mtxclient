#include <gtest/gtest.h>

#include <mtxclient/crypto/client.hpp>
#include <nlohmann/json.hpp>

#include <olm/olm.h>

using json = nlohmann::json;

using namespace mtx::crypto;
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

    EXPECT_EQ(data.dump(),
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

TEST(Utilities, VerifySignedOneTimeKey)
{
    auto alice = make_shared<OlmClient>();
    alice->create_new_account();
    alice->create_new_utility();

    alice->generate_one_time_keys(1);
    auto keys = alice->one_time_keys();

    auto first_key = keys.curve25519.begin()->second;
    auto msg       = json({{"key", first_key}}).dump();

    auto sig = alice->sign_message(msg);

    auto res = olm_ed25519_verify(alice->utility(),
                                  alice->identity_keys().ed25519.data(),
                                  alice->identity_keys().ed25519.size(),
                                  msg.data(),
                                  msg.size(),
                                  (void *)sig.data(),
                                  sig.size());

    EXPECT_EQ(std::string(olm_utility_last_error(alice->utility())), "SUCCESS");
    EXPECT_EQ(res, 0);
}

TEST(Utilities, ValidUploadKeysRequest)
{
    const std::string user_id   = "@alice:matrix.org";
    const std::string device_id = "FKALSOCCC";

    auto alice = make_shared<OlmClient>();
    alice->create_new_account();
    alice->set_device_id(device_id);
    alice->set_user_id(user_id);
    alice->generate_one_time_keys(1);

    auto id_sig = alice->sign_identity_keys();

    json body{{"algorithms", {"m.olm.v1.curve25519-aes-sha2", "m.megolm.v1.aes-sha2"}},
              {"user_id", user_id},
              {"device_id", device_id},
              {"keys",
               {
                 {"curve25519:" + device_id, alice->identity_keys().curve25519},
                 {"ed25519:" + device_id, alice->identity_keys().ed25519},
               }}};

    body["signatures"][user_id]["ed25519:" + device_id] = id_sig;

    json obj         = alice->create_upload_keys_request();
    json device_keys = obj.at("device_keys");

    ASSERT_TRUE(device_keys.dump() == body.dump());

    ASSERT_TRUE(verify_identity_signature(
      body.get<mtx::crypto::DeviceKeys>(), DeviceId(device_id), UserId(user_id)));
    ASSERT_TRUE(verify_identity_signature(
      device_keys.get<mtx::crypto::DeviceKeys>(), DeviceId(device_id), UserId(user_id)));
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

    auto sig = alice->sign_message(msg);

    auto res = olm_ed25519_verify(alice->utility(),
                                  alice->identity_keys().ed25519.data(),
                                  alice->identity_keys().ed25519.size(),
                                  msg.data(),
                                  msg.size(),
                                  (void *)sig.data(),
                                  sig.size());

    EXPECT_EQ(std::string(olm_utility_last_error(alice->utility())), "SUCCESS");
    EXPECT_EQ(res, 0);
}

TEST(Utilities, VerifySignature)
{
    auto alice = make_shared<OlmClient>();
    alice->create_new_account();
    alice->create_new_utility();

    json keys = alice->identity_keys();

    auto j = json(
      {{"keys", {{"ed25519:" + alice->identity_keys().ed25519, alice->identity_keys().ed25519}}},
       {"usage", {"master_key"}},
       {"user_id", "@alice:localhost"}});

    mtx::crypto::CrossSigningKeys msg;

    msg.user_id                                           = "@alice:localhost";
    msg.usage                                             = {"master_key"};
    msg.keys["ed25519:" + alice->identity_keys().ed25519] = alice->identity_keys().ed25519;

    auto sig = alice->sign_message(j.dump());
    auto j1  = json(msg);
    j1.erase("signatures");
    auto sig1 = alice->sign_message(j1.dump());

    EXPECT_EQ(mtx::crypto::ed25519_verify_signature(alice->identity_keys().ed25519, j1, sig), true);
    EXPECT_EQ(
      mtx::crypto::ed25519_verify_signature(alice->identity_keys().ed25519, json(msg), sig1), true);
}

TEST(Utilities, VerifyIdentityKeyJson)
{
    //! JSON extracted from an account created through Riot.
    json data = R"({
	"algorithms": [
	  "m.olm.v1.curve25519-aes-sha2",
          "m.megolm.v1.aes-sha2"
        ],
        "device_id": "VVLXGGTJGN",
        "keys": {
          "curve25519:VVLXGGTJGN": "TEdjuBVstvGMy0NYJxpeD7Zo97bLEgT2ukefWDPbe0w",
          "ed25519:VVLXGGTJGN": "L5IUXmjZGzZO9IwB/j61lTjuD79TCMRDM4bBHvGstT4"
        },
        "signatures": {
          "@nheko_test:matrix.org": {
            "ed25519:VVLXGGTJGN": "tVWnGmZ5cMHiLJiaMhkZjNThQXlvFBsal3dclgPyiqkm/dG7F65U8xHpRb3QWFWALo9iy+L7W+fwv0yGhJFxBQ"
          }
        },
        "unsigned": {
          "device_display_name": "https://riot.im/develop/ via Firefox on Linux"
        },
        "user_id": "@nheko_test:matrix.org"
        })"_json;

    const auto user_id   = data.at("user_id").get<std::string>();
    const auto device_id = data.at("device_id").get<std::string>();

    ASSERT_TRUE(verify_identity_signature(
      data.get<mtx::crypto::DeviceKeys>(), DeviceId(device_id), UserId(user_id)));
}
