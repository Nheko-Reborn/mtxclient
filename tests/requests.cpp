#include <gtest/gtest.h>

#include <nlohmann/json.hpp>

#include <mtx/requests.hpp>
#include <mtx/user_interactive.hpp>

using json = nlohmann::json;
using namespace mtx::requests;

TEST(Requests, Login)
{
    Login t1, t2, t3;

    t1.identifier                  = login_identifier::User{"@alice:matrix.org"};
    t1.password                    = "secret";
    t1.initial_device_display_name = "Mobile";

    json j = t1;
    ASSERT_EQ(j.dump(),
              "{\"identifier\":{\"type\":\"m.id.user\",\"user\":\"@alice:matrix.org\"},"
              "\"initial_device_display_name\":\"Mobile\",\"password\":\"secret\",\"type\":"
              "\"m.login.password\"}");

    t2.identifier = login_identifier::User{"@bob:matrix.org"};
    t2.password   = "secret2";

    j = t2;
    ASSERT_EQ(j.dump(),
              "{\"identifier\":{\"type\":\"m.id.user\",\"user\":\"@bob:matrix.org\"},"
              "\"password\":\"secret2\",\"type\":\"m.login.password\"}");

    t3.identifier = login_identifier::User{"@carl:matrix.org"};
    t3.password   = "secret3";
    t3.device_id  = "ZSDF2RG";

    j = t3;
    ASSERT_EQ(j.dump(),
              "{\"device_id\":\"ZSDF2RG\",\"identifier\":{\"type\":\"m.id.user\",\"user\":\""
              "@carl:matrix.org\"},\"password\":\"secret3\",\"type\":\"m.login."
              "password\"}");
}

TEST(Requests, Typing)
{
    TypingNotification t1, t2;

    t1.timeout = 4000;

    json j = t1;
    ASSERT_EQ(j.dump(), "{\"timeout\":4000,\"typing\":false}");

    t2.typing  = true;
    t2.timeout = 4242;

    j = t2;
    ASSERT_EQ(j.dump(), "{\"timeout\":4242,\"typing\":true}");
}

TEST(Requests, UploadKeys)
{
    UploadKeys r1, r2, r3;

    json j = r1;
    ASSERT_EQ(j.dump(), "{}");

    r2.device_keys.user_id   = "@alice:example.com";
    r2.device_keys.device_id = "JLAFKJWSCS";
    r2.device_keys.keys.emplace("curve25519:JLAFKJWSCS",
                                "3C5BFWi2Y8MaVvjM8M22DBmh24PmgR0nPvJOIArzgyI");
    std::map<std::string, std::string> tmp = {
      {"ed25519:JLAFKJWSCS",
       "dSO80A01XiigH3uBiDVx/EjzaoycHcjq9lfQX0uWsqxl2giMIiSPR8a4d291W1ihKJL/"
       "a+myXS367WT6NAIcBA"}};
    r2.device_keys.signatures.emplace("@alice:example.com", tmp);

    // Only device_keys are present
    j = r2;
    ASSERT_EQ(j.dump(),
              "{\"device_keys\":{\"algorithms\":[\"m.olm.v1.curve25519-aes-sha2\",\"m.megolm.v1."
              "aes-sha2\"],\"device_id\":\"JLAFKJWSCS\",\"keys\":{\"curve25519:JLAFKJWSCS\":"
              "\"3C5BFWi2Y8MaVvjM8M22DBmh24PmgR0nPvJOIArzgyI\"},\"signatures\":{\"@alice:"
              "example.com\":{\"ed25519:JLAFKJWSCS\":\"dSO80A01XiigH3uBiDVx/"
              "EjzaoycHcjq9lfQX0uWsqxl2giMIiSPR8a4d291W1ihKJL/"
              "a+myXS367WT6NAIcBA\"}},\"user_id\":\"@alice:example.com\"}}");

    mtx::requests::SignedOneTimeKey k1;
    k1.key        = "zKbLg+NrIjpnagy+pIY6uPL4ZwEG2v+8F9lmgsnlZzs";
    k1.signatures = {{"@alice:example.com",
                      {{"ed25519:JLAFKJWSCS",
                        "IQeCEPb9HFk217cU9kw9EOiusC6kMIkoIRnbnfOh5Oc63S1ghgyjShBGpu34blQomoalCy"
                        "XWyhaaT3MrLZYQAA"}}}};

    mtx::requests::SignedOneTimeKey k2;
    k2.key = "j3fR3HemM16M7CWhoI4Sk5ZsdmdfQHsKL1xuSft6MSw";
    k2.signatures["@alice:example.com"]["ed25519:JLAFKJWSCS"] =
      "FLWxXqGbwrb8SM3Y795eB6OA8bwBcoMZFXBqnTn58AYWZSqiD45tlBVcDa2L7RwdKXebW/VzDlnfVJ+9jok1Bw";

    r3.one_time_keys.emplace("curve25519:AAAAAQ", "/qyvZvwjiTxGdGU0RCguDCLeR+nmsb3FfNG3/Ve4vU8");
    r3.one_time_keys.emplace("signed_curve25519:AAAAHg", k1);
    r3.one_time_keys.emplace("signed_curve25519:AAAAHQ", k2);

    j = r3;
    ASSERT_EQ(
      j.dump(),
      "{\"one_time_keys\":{\"curve25519:AAAAAQ\":\"/qyvZvwjiTxGdGU0RCguDCLeR+nmsb3FfNG3/"
      "Ve4vU8\",\"signed_curve25519:AAAAHQ\":{\"key\":"
      "\"j3fR3HemM16M7CWhoI4Sk5ZsdmdfQHsKL1xuSft6MSw\",\"signatures\":{\"@alice:example.com\":{"
      "\"ed25519:JLAFKJWSCS\":"
      "\"FLWxXqGbwrb8SM3Y795eB6OA8bwBcoMZFXBqnTn58AYWZSqiD45tlBVcDa2L7RwdKXebW/"
      "VzDlnfVJ+9jok1Bw\"}}},\"signed_curve25519:AAAAHg\":{\"key\":\"zKbLg+NrIjpnagy+"
      "pIY6uPL4ZwEG2v+8F9lmgsnlZzs\",\"signatures\":{\"@alice:example.com\":{\"ed25519:"
      "JLAFKJWSCS\":"
      "\"IQeCEPb9HFk217cU9kw9EOiusC6kMIkoIRnbnfOh5Oc63S1ghgyjShBGpu34blQomoalCyXWyhaaT3MrLZYQAA"
      "\"}}}}}");

    k1.fallback = true;
    r3.fallback_keys.emplace("signed_curve25519:AAAAAA", k1);
    j = r3;

    ASSERT_EQ(
      j.dump(),
      "{\"fallback_keys\":{\"signed_curve25519:AAAAAA\":{\"fallback\":true,\"key\":\"zKbLg+"
      "NrIjpnagy+pIY6uPL4ZwEG2v+8F9lmgsnlZzs\",\"signatures\":{\"@alice:example.com\":{\"ed25519:"
      "JLAFKJWSCS\":"
      "\"IQeCEPb9HFk217cU9kw9EOiusC6kMIkoIRnbnfOh5Oc63S1ghgyjShBGpu34blQomoalCyXWyhaaT3MrLZYQAA\"}}"
      "}},\"one_time_keys\":{\"curve25519:AAAAAQ\":\"/qyvZvwjiTxGdGU0RCguDCLeR+nmsb3FfNG3/"
      "Ve4vU8\",\"signed_curve25519:AAAAHQ\":{\"key\":"
      "\"j3fR3HemM16M7CWhoI4Sk5ZsdmdfQHsKL1xuSft6MSw\",\"signatures\":{\"@alice:example.com\":{"
      "\"ed25519:JLAFKJWSCS\":"
      "\"FLWxXqGbwrb8SM3Y795eB6OA8bwBcoMZFXBqnTn58AYWZSqiD45tlBVcDa2L7RwdKXebW/"
      "VzDlnfVJ+9jok1Bw\"}}},\"signed_curve25519:AAAAHg\":{\"key\":\"zKbLg+NrIjpnagy+"
      "pIY6uPL4ZwEG2v+8F9lmgsnlZzs\",\"signatures\":{\"@alice:example.com\":{\"ed25519:"
      "JLAFKJWSCS\":"
      "\"IQeCEPb9HFk217cU9kw9EOiusC6kMIkoIRnbnfOh5Oc63S1ghgyjShBGpu34blQomoalCyXWyhaaT3MrLZYQAA\"}}"
      "}}}");
}

TEST(Requests, QueryKeys)
{
    QueryKeys k1;

    std::vector<std::string> empty_vec;

    k1.device_keys.emplace("@alice:localhost", empty_vec);
    k1.token = "this_is_a_token";

    json j = k1;
    ASSERT_EQ(j.dump(),
              "{\"device_keys\":{\"@alice:localhost\":[]},\"timeout\":10000,\"token\":\"this_"
              "is_a_token\"}");
}

TEST(Requests, ClaimKeys)
{
    ClaimKeys k1;

    std::map<std::string, std::string> devices;
    devices.emplace("JLAFKJWSCS", "curve25519");

    k1.one_time_keys.emplace("@alice:localhost", devices);

    json j = k1;
    ASSERT_EQ(j.dump(),
              "{\"one_time_keys\":{\"@alice:localhost\":{\"JLAFKJWSCS\":\"curve25519\"}},"
              "\"timeout\":10000}");
}

TEST(Requests, UserInteractiveAuth)
{
    using namespace mtx::user_interactive;
    // Auth pw{
    //  .session = "<session ID>",
    //  .content = auth::Password{.password        = "<password>",
    //                            .identifier_type = auth::Password::IdType::UserId,
    //                            .identifier_user = "<user_id or user localpart>"},
    //};
    Auth a;
    a.session = "<session ID>";
    auth::Password pw_content;
    pw_content.password        = "<password>";
    pw_content.identifier_type = auth::Password::IdType::UserId;
    pw_content.identifier_user = "<user_id or user localpart>";
    a.content                  = pw_content;

    EXPECT_EQ(nlohmann::json(a), R"({
  "type": "m.login.password",
  "identifier": {
    "type": "m.id.user",
    "user": "<user_id or user localpart>"
  },
  "password": "<password>",
  "session": "<session ID>"
})"_json);

    pw_content.identifier_type    = auth::Password::IdType::ThirdPartyId;
    pw_content.identifier_user    = "";
    pw_content.identifier_medium  = "<The medium of the third party identifier.>";
    pw_content.identifier_address = "<The third party address of the user>";
    a.content                     = pw_content;

    EXPECT_EQ(nlohmann::json(a), R"({
  "type": "m.login.password",
  "identifier": {
    "type": "m.id.thirdparty",
    "medium": "<The medium of the third party identifier.>",
    "address": "<The third party address of the user>"
  },
  "password": "<password>",
  "session": "<session ID>"
})"_json);

    a.content = auth::ReCaptcha{"<captcha response>"};

    EXPECT_EQ(nlohmann::json(a), R"({
  "type": "m.login.recaptcha",
  "response": "<captcha response>",
  "session": "<session ID>"
})"_json);

    a.content = auth::Token{"<token>", "<client generated nonce>"};
    EXPECT_EQ(nlohmann::json(a), R"({
  "type": "m.login.token",
  "token": "<token>",
  "txn_id": "<client generated nonce>",
  "session": "<session ID>"
})"_json);

    a.content = auth::OAuth2{};
    EXPECT_EQ(nlohmann::json(a), R"({
  "type": "m.login.oauth2",
  "session": "<session ID>"
})"_json);
    a.content = auth::Terms{};
    EXPECT_EQ(nlohmann::json(a), R"({
  "type": "m.login.terms",
  "session": "<session ID>"
})"_json);
    a.content = auth::Dummy{};
    EXPECT_EQ(nlohmann::json(a), R"({
  "type": "m.login.dummy",
  "session": "<session ID>"
})"_json);
    a.content = auth::Fallback{};
    EXPECT_EQ(nlohmann::json(a), R"({
  "session": "<session ID>"
})"_json);

    a.content =
      auth::EmailIdentity{"<identity server session id>",
                          "<identity server client secret>",
                          "<url of identity server authed with, e.g. 'matrix.org:8090'>",
                          "<access token previously registered with the identity server>"};

    EXPECT_EQ(nlohmann::json(a), R"({
  "type": "m.login.email.identity",
  "threepid_creds": {
    "sid": "<identity server session id>",
    "client_secret": "<identity server client secret>",
    "id_server": "<url of identity server authed with, e.g. 'matrix.org:8090'>",
    "id_access_token": "<access token previously registered with the identity server>"
  },
  "session": "<session ID>"
})"_json);

    a.content = auth::MSISDN{"<identity server session id>",
                             "<identity server client secret>",
                             "<url of identity server authed with, e.g. 'matrix.org:8090'>",
                             "<access token previously registered with the identity server>"};

    EXPECT_EQ(nlohmann::json(a), R"({
  "type": "m.login.msisdn",
  "threepid_creds": {
    "sid": "<identity server session id>",
    "client_secret": "<identity server client secret>",
    "id_server": "<url of identity server authed with, e.g. 'matrix.org:8090'>",
    "id_access_token": "<access token previously registered with the identity server>"
  },
  "session": "<session ID>"
})"_json);

    a.content =
      auth::MSISDN{"<identity server session id>", "<identity server client secret>", "", ""};

    EXPECT_EQ(nlohmann::json(a), R"({
  "type": "m.login.msisdn",
  "threepid_creds": {
    "sid": "<identity server session id>",
    "client_secret": "<identity server client secret>"
  },
  "session": "<session ID>"
})"_json);

    a.content = auth::RegistrationToken{"<token>"};
    EXPECT_EQ(nlohmann::json(a), R"({
  "type": "m.login.registration_token",
  "token": "<token>",
  "session": "<session ID>"
})"_json);
}

TEST(Requests, RoomVisibility)
{
    mtx::requests::PublicRoomVisibility req;
    req.visibility = mtx::common::RoomVisibility::Private;
    json j         = req;
    EXPECT_EQ(j, R"({
    "visibility" : "private"
  })"_json);

    req.visibility = mtx::common::RoomVisibility::Public;
    j              = req;
    EXPECT_EQ(j, R"({
    "visibility" : "public"
  })"_json);
}

TEST(Requests, PublicRooms)
{
    PublicRooms b1, b2, b3;

    b1.limit                      = 10;
    b1.filter.generic_search_term = "foo";
    b1.include_all_networks       = false;
    b1.third_party_instance_id    = "irc";

    json j = b1;

    EXPECT_EQ(j, R"({
    "limit" : 10,
    "filter" : {
      "generic_search_term" : "foo"
    },
    "include_all_networks" : false,
    "third_party_instance_id" : "irc"
  })"_json);

    // if third_party_instance_id is set, then the include_all_networks flag should
    // default to false
    b2.limit                   = 10;
    b2.third_party_instance_id = "matrix";
    j                          = b2;
    EXPECT_EQ(j, R"({
    "limit" : 10,
    "include_all_networks" : false,
    "third_party_instance_id" : "matrix"
  })"_json);

    // if include_all_networks is true, then third_party_instance_id cannot be used.
    // if it is somehow set, then we expect an exception to be thrown
    b3.limit                   = 2;
    b3.include_all_networks    = true;
    b3.third_party_instance_id = "irc";

    EXPECT_THROW(json req = b3, std::invalid_argument);
}
