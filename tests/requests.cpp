#include <gtest/gtest.h>

#include <nlohmann/json.hpp>

#include <mtx/requests.hpp>

using json = nlohmann::json;
using namespace mtx::requests;

TEST(Requests, Login)
{
        Login t1, t2, t3;

        t1.user                        = "@alice:matrix.org";
        t1.password                    = "secret";
        t1.initial_device_display_name = "Mobile";

        json j = t1;
        ASSERT_EQ(j.dump(),
                  "{\"initial_device_display_name\":\"Mobile\",\"password\":\"secret\",\"type\":"
                  "\"m.login.password\",\"user\":\"@alice:matrix.org\"}");

        t2.user     = "@bob:matrix.org";
        t2.password = "secret2";

        j = t2;
        ASSERT_EQ(
          j.dump(),
          "{\"password\":\"secret2\",\"type\":\"m.login.password\",\"user\":\"@bob:matrix.org\"}");

        t3.user      = "@carl:matrix.org";
        t3.password  = "secret3";
        t3.device_id = "ZSDF2RG";

        j = t3;
        ASSERT_EQ(j.dump(),
                  "{\"device_id\":\"ZSDF2RG\",\"password\":\"secret3\",\"type\":\"m.login."
                  "password\",\"user\":\"@carl:matrix.org\"}");
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
        ASSERT_EQ(
          j.dump(),
          "{\"device_keys\":{\"algorithms\":[\"m.olm.v1.curve25519-aes-sha2\",\"m.megolm.v1."
          "aes-sha2\"],\"device_id\":\"JLAFKJWSCS\",\"keys\":{\"curve25519:JLAFKJWSCS\":"
          "\"3C5BFWi2Y8MaVvjM8M22DBmh24PmgR0nPvJOIArzgyI\"},\"signatures\":{\"@alice:"
          "example.com\":{\"ed25519:JLAFKJWSCS\":\"dSO80A01XiigH3uBiDVx/"
          "EjzaoycHcjq9lfQX0uWsqxl2giMIiSPR8a4d291W1ihKJL/"
          "a+myXS367WT6NAIcBA\"}},\"user_id\":\"@alice:example.com\"}}");

        json k1 = {{"key", "zKbLg+NrIjpnagy+pIY6uPL4ZwEG2v+8F9lmgsnlZzs"},
                   {"signatures",
                    {{"@alice:example.com",
                      {{"ed25519:JLAFKJWSCS",
                        "IQeCEPb9HFk217cU9kw9EOiusC6kMIkoIRnbnfOh5Oc63S1ghgyjShBGpu34blQomoalCyXWyh"
                        "aaT3MrLZYQ"
                        "AA"}}}}}};

        json k2 = {{"key", "j3fR3HemM16M7CWhoI4Sk5ZsdmdfQHsKL1xuSft6MSw"},
                   {"signatures",
                    {{"@alice:example.com",
                      {{"ed25519:JLAFKJWSCS",
                        "FLWxXqGbwrb8SM3Y795eB6OA8bwBcoMZFXBqnTn58AYWZSqiD45tlBVcDa2L7RwdKXebW/"
                        "VzDlnfVJ+9jok1Bw"}}}}}};

        r3.one_time_keys.emplace("curve25519:AAAAAQ",
                                 "/qyvZvwjiTxGdGU0RCguDCLeR+nmsb3FfNG3/Ve4vU8");
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
