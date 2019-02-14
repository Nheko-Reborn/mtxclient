#include <gtest/gtest.h>
#include <nlohmann/json.hpp>

#include <mtx/common.hpp>

using json = nlohmann::json;

using namespace mtx::crypto;

TEST(Crypto, DeviceKeys)
{
        DeviceKeys device1;

        device1.user_id   = "@alice:example.com";
        device1.device_id = "JLAFKJWSCS";
        device1.keys = {{"curve25519:JLAFKJWSCS", "3C5BFWi2Y8MaVvjM8M22DBmh24PmgR0nPvJOIArzgyI"},
                        {"ed25519:JLAFKJWSCS", "lEuiRJBit0IG6nUf5pUzWTUEsRVVe/HJkoKuEww9ULI"}};
        device1.signatures = {
          {"@alice:example.com",
           {{"ed25519:JLAFKJWSCS",
             "dSO80A01XiigH3uBiDVx/EjzaoycHcjq9lfQX0uWsqxl2giMIiSPR8a4d291W1ihKJL/"
             "a+myXS367WT6NAIcBA"}}}};

        json j = device1;
        ASSERT_EQ(j.dump(),
                  "{\"algorithms\":[\"m.olm.v1.curve25519-aes-sha2\",\"m.megolm.v1.aes-sha2\"],"
                  "\"device_id\":\"JLAFKJWSCS\",\"keys\":{\"curve25519:JLAFKJWSCS\":"
                  "\"3C5BFWi2Y8MaVvjM8M22DBmh24PmgR0nPvJOIArzgyI\",\"ed25519:JLAFKJWSCS\":"
                  "\"lEuiRJBit0IG6nUf5pUzWTUEsRVVe/"
                  "HJkoKuEww9ULI\"},\"signatures\":{\"@alice:example.com\":{\"ed25519:JLAFKJWSCS\":"
                  "\"dSO80A01XiigH3uBiDVx/EjzaoycHcjq9lfQX0uWsqxl2giMIiSPR8a4d291W1ihKJL/"
                  "a+myXS367WT6NAIcBA\"}},\"user_id\":\"@alice:example.com\"}");

        json data = R"({
          "user_id": "@alice:example.com",
          "device_id": "JLAFKJWSCS",
          "algorithms": [
            "m.olm.v1.curve25519-aes-sha2",
            "m.megolm.v1.aes-sha2"
          ],
          "keys": {
            "curve25519:JLAFKJWSCS": "3C5BFWi2Y8MaVvjM8M22DBmh24PmgR0nPvJOIArzgyI",
            "ed25519:JLAFKJWSCS": "lEuiRJBit0IG6nUf5pUzWTUEsRVVe/HJkoKuEww9ULI"
          },
          "signatures": {
            "@alice:example.com": {
              "ed25519:JLAFKJWSCS": "dSO80A01XiigH3uBiDVx/EjzaoycHcjq9lfQX0uWsqxl2giMIiSPR8a4d291W1ihKJL/a+myXS367WT6NAIcBA"
            }
          },
          "unsigned": {
            "device_display_name": "Alice's mobile phone"
          }
        })"_json;

        DeviceKeys device2 = data;

        EXPECT_EQ(device2.user_id, device1.user_id);
        EXPECT_EQ(device2.device_id, device1.device_id);
        EXPECT_EQ(device2.keys, device1.keys);
        EXPECT_EQ(device2.algorithms, device1.algorithms);
        EXPECT_EQ(device2.signatures, device1.signatures);

        EXPECT_EQ(device2.unsigned_info.device_display_name, "Alice's mobile phone");
}
