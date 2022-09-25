#include <gtest/gtest.h>
#include <nlohmann/json.hpp>

#include "mtxclient/crypto/client.hpp"
#include "mtxclient/crypto/types.hpp"
#include "mtxclient/http/client.hpp"

#include "mtx/requests.hpp"
#include "mtx/responses.hpp"
#include "mtx/secret_storage.hpp"

using namespace mtx::client;
using namespace mtx::http;
using namespace mtx::crypto;

using namespace mtx::identifiers;
using namespace mtx::events;
using namespace mtx::events::msg;
using namespace mtx::events::collections;
using namespace mtx::responses;

using namespace std;

using namespace nlohmann;

TEST(Crypto, DeviceKeys)
{
    DeviceKeys device1;

    device1.user_id    = "@alice:example.com";
    device1.device_id  = "JLAFKJWSCS";
    device1.keys       = {{"curve25519:JLAFKJWSCS", "3C5BFWi2Y8MaVvjM8M22DBmh24PmgR0nPvJOIArzgyI"},
                          {"ed25519:JLAFKJWSCS", "lEuiRJBit0IG6nUf5pUzWTUEsRVVe/HJkoKuEww9ULI"}};
    device1.signatures = {{"@alice:example.com",
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

    DeviceKeys device2 = data.get<DeviceKeys>();

    EXPECT_EQ(device2.user_id, device1.user_id);
    EXPECT_EQ(device2.device_id, device1.device_id);
    EXPECT_EQ(device2.keys, device1.keys);
    EXPECT_EQ(device2.algorithms, device1.algorithms);
    EXPECT_EQ(device2.signatures, device1.signatures);

    EXPECT_EQ(device2.unsigned_info.device_display_name, "Alice's mobile phone");
}

TEST(Crypto, EncryptedFile)
{
    json j = R"({
      "url": "mxc://example.org/FHyPlCeYUSFFxlgbQYZmoEoe",
      "v": "v2",
      "key": {
        "alg": "A256CTR",
        "ext": true,
        "k": "aWF6-32KGYaC3A_FEUCk1Bt0JA37zP0wrStgmdCaW-0",
        "key_ops": ["encrypt","decrypt"],
        "kty": "oct"
      },
      "iv": "w+sE15fzSc0AAAAAAAAAAA",
      "hashes": {
        "sha256": "fdSLu/YkRx3Wyh3KQabP3rd6+SFiKg5lsJZQHtkSAYA"
      }})"_json;

    EncryptedFile file = j.get<EncryptedFile>();
    // json j2            = file;

    // EXPECT_EQ(j, j2);
    EXPECT_EQ(file.v, "v2");
    EXPECT_EQ(file.iv, "w+sE15fzSc0AAAAAAAAAAA");
    EXPECT_EQ(file.hashes.at("sha256"), "fdSLu/YkRx3Wyh3KQabP3rd6+SFiKg5lsJZQHtkSAYA");
    EXPECT_EQ(file.key.alg, "A256CTR");
    EXPECT_EQ(file.key.ext, true);
    EXPECT_EQ(file.key.k, "aWF6-32KGYaC3A_FEUCk1Bt0JA37zP0wrStgmdCaW-0");
    EXPECT_EQ(file.key.key_ops.size(), 2);
    EXPECT_EQ(file.key.kty, "oct");
}

TEST(Base64, EncodingDecoding)
{
    std::string random_str =
      "+7TE+9qmFWHPnrBLd03MtoXsRlhYaQt2tLBg4kZJI+NFcXVxqNUI1S3c97eV8aVgSj1/"
      "eo8PsnRNO29c2TgPLXvah2GDl90ehHjzH/"
      "vMBJKPdqyE31ch7NYBgvLBVoesrRyDoIYDlbRhHiRDTmLKMC55WN1YvDJu2Pvg3WxZiANobk"
      "0EPzHABqOYLaYiVxFrdko7mm8pDZXlatys+dvLv9Zf6lxfd/5MPK1C52m/UhnrZ3shS/"
      "XBzxRfBikZQjl7C9IMo7l170ffipN8QHb5LmZlj4V41DUJHCU=";

    EXPECT_EQ(base642bin(bin2base64(random_str)), random_str);
    EXPECT_EQ(bin2base64(base642bin(random_str)), random_str);
    EXPECT_EQ(base642bin_unpadded(bin2base64_unpadded(random_str)), random_str);
    // EXPECT_EQ(bin2base64_unpadded(base642bin_unpadded(random_str)), random_str);
    EXPECT_EQ(base642bin_urlsafe_unpadded(bin2base64_urlsafe_unpadded(random_str)), random_str);
    // EXPECT_EQ(bin2base64_urlsafe_unpadded(base642bin_urlsafe_unpadded(random_str)),
    // random_str);

    EXPECT_EQ(bin2base64(""), "");
    EXPECT_EQ(bin2base64("f"), "Zg==");
    EXPECT_EQ(bin2base64("fo"), "Zm8=");
    EXPECT_EQ(bin2base64("foo"), "Zm9v");
    EXPECT_EQ(bin2base64("foob"), "Zm9vYg==");
    EXPECT_EQ(bin2base64("fooba"), "Zm9vYmE=");
    EXPECT_EQ(bin2base64("foobar"), "Zm9vYmFy");

    EXPECT_EQ("", base642bin(""));
    EXPECT_EQ("f", base642bin("Zg=="));
    EXPECT_EQ("fo", base642bin("Zm8="));
    EXPECT_EQ("foo", base642bin("Zm9v"));
    EXPECT_EQ("foob", base642bin("Zm9vYg=="));
    EXPECT_EQ("fooba", base642bin("Zm9vYmE="));
    EXPECT_EQ("foobar", base642bin("Zm9vYmFy"));

    EXPECT_EQ(bin2base64_unpadded(""), "");
    EXPECT_EQ(bin2base64_unpadded("f"), "Zg");
    EXPECT_EQ(bin2base64_unpadded("fo"), "Zm8");
    EXPECT_EQ(bin2base64_unpadded("foo"), "Zm9v");
    EXPECT_EQ(bin2base64_unpadded("foob"), "Zm9vYg");
    EXPECT_EQ(bin2base64_unpadded("fooba"), "Zm9vYmE");
    EXPECT_EQ(bin2base64_unpadded("foobar"), "Zm9vYmFy");

    EXPECT_EQ("", base642bin_unpadded(""));
    EXPECT_EQ("f", base642bin_unpadded("Zg=="));
    EXPECT_EQ("fo", base642bin_unpadded("Zm8="));
    EXPECT_EQ("foo", base642bin_unpadded("Zm9v"));
    EXPECT_EQ("foob", base642bin_unpadded("Zm9vYg=="));
    EXPECT_EQ("fooba", base642bin_unpadded("Zm9vYmE="));
    EXPECT_EQ("foobar", base642bin_unpadded("Zm9vYmFy"));

    EXPECT_EQ(bin2base64_urlsafe_unpadded(""), "");
    EXPECT_EQ(bin2base64_urlsafe_unpadded("f"), "Zg");
    EXPECT_EQ(bin2base64_urlsafe_unpadded("fo"), "Zm8");
    EXPECT_EQ(bin2base64_urlsafe_unpadded("foo"), "Zm9v");
    EXPECT_EQ(bin2base64_urlsafe_unpadded("foob"), "Zm9vYg");
    EXPECT_EQ(bin2base64_urlsafe_unpadded("fooba"), "Zm9vYmE");
    EXPECT_EQ(bin2base64_urlsafe_unpadded("foobar"), "Zm9vYmFy");

    EXPECT_EQ("", base642bin_urlsafe_unpadded(""));
    EXPECT_EQ("f", base642bin_urlsafe_unpadded("Zg=="));
    EXPECT_EQ("fo", base642bin_urlsafe_unpadded("Zm8="));
    EXPECT_EQ("foo", base642bin_urlsafe_unpadded("Zm9v"));
    EXPECT_EQ("foob", base642bin_urlsafe_unpadded("Zm9vYg=="));
    EXPECT_EQ("fooba", base642bin_urlsafe_unpadded("Zm9vYmE="));
    EXPECT_EQ("foobar", base642bin_urlsafe_unpadded("Zm9vYmFy"));
}

TEST(Base58, EncodingDecoding)
{
    EXPECT_EQ(bin2base58(""), "");
    EXPECT_EQ(bin2base58("f"), "2m");
    EXPECT_EQ(bin2base58("fo"), "8o8");
    EXPECT_EQ(bin2base58("foo"), "bQbp");
    EXPECT_EQ(bin2base58("foob"), "3csAg9");
    EXPECT_EQ(bin2base58("fooba"), "CZJRhmz");
    EXPECT_EQ(bin2base58("foobar"), "t1Zv2yaZ");
    EXPECT_FALSE(bin2base58(to_string(create_buffer(32))).empty());

    EXPECT_EQ("", base582bin(""));
    EXPECT_EQ("f", base582bin("2m"));
    EXPECT_EQ("fo", base582bin("8o8"));
    EXPECT_EQ("foo", base582bin("bQbp"));
    EXPECT_EQ("foob", base582bin("3csAg9"));
    EXPECT_EQ("fooba", base582bin("CZJRhmz"));
    EXPECT_EQ("foobar", base582bin("t1Zv2yaZ"));
}

TEST(ExportSessions, EncryptDecrypt)
{
    constexpr auto PASS = "secret_passphrase";

    ExportedSession s1;
    s1.room_id     = "!room_id:example.org";
    s1.session_id  = "sid";
    s1.session_key = "skey";

    ExportedSessionKeys keys;
    keys.sessions = {s1, s1, s1};

    std::string ciphertext = mtx::crypto::encrypt_exported_sessions(keys, PASS);
    EXPECT_TRUE(ciphertext.size() > 0);

    auto encoded = bin2base64(ciphertext);

    auto restored_keys = mtx::crypto::decrypt_exported_sessions(encoded, PASS);
    EXPECT_EQ(json(keys).dump(), json(restored_keys).dump());
}

TEST(Encryption, EncryptedFile)
{
    {
        auto buffer = mtx::crypto::create_buffer(16);
        ASSERT_EQ(buffer.size(), 16);
        auto buf_str = mtx::crypto::to_string(buffer);
        ASSERT_EQ(buf_str.size(), 16);
    }

    std::string plaintext = "This is some plain text payload";
    auto encryption_data  = mtx::crypto::encrypt_file(plaintext);
    ASSERT_NE(plaintext, mtx::crypto::to_string(encryption_data.first));
    ASSERT_EQ(plaintext,
              mtx::crypto::to_string(mtx::crypto::decrypt_file(
                mtx::crypto::to_string(encryption_data.first), encryption_data.second)));
    // key needs to be 32 bytes/256 bits
    ASSERT_EQ(32, mtx::crypto::base642bin_urlsafe_unpadded(encryption_data.second.key.k).size());
    // IV needs to be 16 bytes/128 bits
    ASSERT_EQ(16, mtx::crypto::base642bin_unpadded(encryption_data.second.iv).size());
    ASSERT_EQ(16, mtx::crypto::base642bin_unpadded(encryption_data.second.iv).size());
    EXPECT_EQ(mtx::crypto::bin2base64_unpadded(std::string(8, '\0')),
              encryption_data.second.iv.substr(11));
    auto iv = mtx::crypto::base642bin_unpadded(encryption_data.second.iv);
    EXPECT_EQ(std::vector<uint8_t>(8, 0), std::vector<uint8_t>(iv.begin() + 8, iv.end()));
    EXPECT_NE("AAAAAAAAAAAA", encryption_data.second.iv.substr(0, 11));
    EXPECT_NE(std::vector<uint8_t>(8, 0), std::vector<uint8_t>(iv.begin(), iv.begin() + 8));

    json j = R"({
  "type": "m.room.message",
  "content": {
    "body": "test.txt",
    "info": {
      "size": 8,
      "mimetype": "text/plain"
    },
    "msgtype": "m.file",
    "file": {
      "v": "v2",
      "key": {
        "alg": "A256CTR",
        "ext": true,
        "k": "6osKLzUKV1YZ06WEX0b77D784Te8oAj5eNU-gAgkjs4",
        "key_ops": [
          "encrypt",
          "decrypt"
        ],
        "kty": "oct"
      },
      "iv": "7zRP/t89YWcAAAAAAAAAAA",
      "hashes": {
        "sha256": "5g41hn7n10sCw3+2j7CQ9SJl6R/v5EBT4MshdFgHhzo"
      },
      "url": "mxc://neko.dev/WPKoOAPfPlcHiZZTEoaIoZhN",
      "mimetype": "text/plain"
    }
 },
 "event_id": "$1575320135447DEPky:neko.dev",
  "origin_server_ts": 1575320135324,
  "sender": "@test:neko.dev",
  "unsigned": {
    "age": 1081,
    "transaction_id": "m1575320142400.8"
  },
  "room_id": "!YnUlhwgbBaGcAFsJOJ:neko.dev"
})"_json;
    mtx::events::RoomEvent<mtx::events::msg::File> ev =
      j.get<mtx::events::RoomEvent<mtx::events::msg::File>>();

    ASSERT_EQ("abcdefg\n",
              mtx::crypto::to_string(
                mtx::crypto::decrypt_file("=\xFDX\xAB\xCA\xEB\x8F\xFF", ev.content.file.value())));
}

TEST(Encryption, PkEncryptionDecryption)
{
    mtx::responses::backup::SessionData d;
    d.algorithm   = "m.megolm.v1.aes-sha2";
    d.sender_key  = "sender_key";
    d.session_key = "session_key";

    auto privKey = mtx::crypto::create_buffer(256 / 8);

    auto publicKey = mtx::crypto::CURVE25519_public_key_from_private(privKey);
    auto encrypted = mtx::crypto::encrypt_session(d, publicKey);
    auto decrypted = mtx::crypto::decrypt_session(encrypted, privKey);

    EXPECT_EQ(d.algorithm, decrypted.algorithm);
    EXPECT_EQ(d.forwarding_curve25519_key_chain, decrypted.forwarding_curve25519_key_chain);
    EXPECT_EQ(d.sender_claimed_keys, decrypted.sender_claimed_keys);
    EXPECT_EQ(d.sender_key, decrypted.sender_key);
    EXPECT_EQ(d.session_key, decrypted.session_key);
}

TEST(SecretStorage, Secret)
{
    json j = R"({
	  "encrypted": {
	      "key_id": {
		"iv": "16+bytes+base64",
		"ciphertext": "base64+encoded+encrypted+data",
		"mac": "base64+encoded+mac"
	      }
	  }
	})"_json;

    mtx::secret_storage::Secret secret = j.get<mtx::secret_storage::Secret>();

    ASSERT_EQ(json(secret), j);
    ASSERT_EQ(secret.encrypted.size(), 1);
    ASSERT_EQ(secret.encrypted["key_id"].iv, "16+bytes+base64");
    ASSERT_EQ(secret.encrypted["key_id"].ciphertext, "base64+encoded+encrypted+data");
    ASSERT_EQ(secret.encrypted["key_id"].mac, "base64+encoded+mac");
}

TEST(SecretStorage, SecretKey)
{
    json j = R"({
	  "name": "m.default",
	  "algorithm": "m.secret_storage.v1.aes-hmac-sha2",
	  "iv": "random+data",
	  "mac": "mac+of+encrypted+zeros"
	})"_json;

    mtx::secret_storage::AesHmacSha2KeyDescription desc =
      j.get<mtx::secret_storage::AesHmacSha2KeyDescription>();

    ASSERT_EQ(json(desc), j);
    ASSERT_EQ(desc.name, "m.default");
    ASSERT_EQ(desc.algorithm, "m.secret_storage.v1.aes-hmac-sha2");
    ASSERT_EQ(desc.iv, "random+data");
    ASSERT_EQ(desc.mac, "mac+of+encrypted+zeros");

    j = R"({
	  "name": "m.default",
	  "algorithm": "m.secret_storage.v1.aes-hmac-sha2",
	  "passphrase": {
	      "algorithm": "m.pbkdf2",
	      "salt": "MmMsAlty",
	      "iterations": 100000,
	      "bits": 512
	  },
	  "iv": "random+data",
	  "mac": "mac+of+encrypted+zeros",
	  "signatures" : {
	    "@alice:localhost" : {
              "ed25519:adkfajfgaefkdahfzguerhtgduifghes": "ksfjvkrfbnrtnwublrjkgnorthgnrdtjbiortbjdlbiutr"
            }
	  }
	})"_json;

    desc = j.get<mtx::secret_storage::AesHmacSha2KeyDescription>();

    ASSERT_EQ(json(desc), j);
    ASSERT_EQ(desc.name, "m.default");
    ASSERT_EQ(desc.algorithm, "m.secret_storage.v1.aes-hmac-sha2");
    ASSERT_EQ(desc.iv, "random+data");
    ASSERT_EQ(desc.mac, "mac+of+encrypted+zeros");
    ASSERT_EQ(desc.passphrase.has_value(), true);
    ASSERT_EQ(desc.passphrase->algorithm, "m.pbkdf2");
    ASSERT_EQ(desc.passphrase->salt, "MmMsAlty");
    ASSERT_EQ(desc.passphrase->iterations, 100000);
    ASSERT_EQ(desc.passphrase->bits, 512);
    ASSERT_EQ(desc.signatures["@alice:localhost"]["ed25519:adkfajfgaefkdahfzguerhtgduifghes"],
              "ksfjvkrfbnrtnwublrjkgnorthgnrdtjbiortbjdlbiutr");
}

TEST(SecretStorage, CreateSecretKey)
{
    auto ssss1 = mtx::crypto::OlmClient::create_ssss_key();
    ASSERT_TRUE(ssss1.has_value());
    EXPECT_FALSE(ssss1->keyDescription.passphrase.has_value());
    EXPECT_EQ(ssss1->keyDescription.algorithm, "m.secret_storage.v1.aes-hmac-sha2");
    EXPECT_GE(ssss1->keyDescription.iv.length(), 32);
    EXPECT_EQ((ssss1->keyDescription.mac.length() - 1) * 3 / 4, 32);

    EXPECT_EQ(key_from_recoverykey(key_to_recoverykey(ssss1->privateKey), ssss1->keyDescription),
              ssss1->privateKey);

    auto ssss2 = mtx::crypto::OlmClient::create_ssss_key("some passphrase");
    ASSERT_TRUE(ssss2.has_value());
    ASSERT_TRUE(ssss2->keyDescription.passphrase.has_value());
    EXPECT_EQ(ssss2->keyDescription.algorithm, "m.secret_storage.v1.aes-hmac-sha2");
    EXPECT_GE(ssss2->keyDescription.iv.length(), 32);
    EXPECT_EQ((ssss2->keyDescription.mac.length() - 1) * 3 / 4, 32);

    EXPECT_EQ(mtx::crypto::key_from_passphrase("some passphrase", ssss2->keyDescription),
              ssss2->privateKey);
}

TEST(SecretStorage, CreateOnlineKeyBackup)
{
    mtx::crypto::OlmClient account;
    account.create_new_account();

    auto cross = account.create_crosssigning_keys();
    ASSERT_TRUE(cross.has_value());

    auto okb = account.create_online_key_backup(cross->private_master_key);
    ASSERT_TRUE(okb.has_value());

    mtx::responses::backup::SessionData s;
    s.algorithm   = mtx::crypto::MEGOLM_ALGO;
    s.sender_key  = "abc";
    s.session_key = "cde";

    auto enc1 = mtx::crypto::encrypt_session(
      s, json::parse(okb->backupVersion.auth_data)["public_key"].get<std::string>());
    EXPECT_FALSE(enc1.ciphertext.empty());

    auto enc2 = mtx::crypto::encrypt_session(
      s, mtx::crypto::CURVE25519_public_key_from_private(okb->privateKey));
    EXPECT_FALSE(enc2.ciphertext.empty());
}
