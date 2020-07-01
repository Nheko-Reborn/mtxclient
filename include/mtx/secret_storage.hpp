#pragma once

#include <cstdint>
#include <map>
#include <optional>
#include <string>

#include <nlohmann/json.hpp>

namespace mtx {
namespace secret_storage {
namespace secrets {
constexpr const char megolm_backup_v1[]           = "m.megolm_backup.v1";
constexpr const char cross_signing_self_signing[] = "m.cross_signing.self_signing";
constexpr const char cross_signing_user_signing[] = "m.cross_signing.user_signing";
constexpr const char cross_signing_master[]       = "m.cross_signing.master";
}

struct AesHmacSha2EncryptedData
{
        std::string iv;         //! Required. The 16-byte initialization vector, encoded as base64.
        std::string ciphertext; //! Required. The AES-CTR-encrypted data, encoded as base64.
        std::string mac;        //! Required. The MAC, encoded as base64.
};

void
to_json(nlohmann::json &obj, const AesHmacSha2EncryptedData &data);

void
from_json(const nlohmann::json &obj, AesHmacSha2EncryptedData &data);

struct Secret
{
        //! Required. Map from key ID the encrypted data. The exact format for the encrypted data is
        //! dependent on the key algorithm. See the definition of AesHmacSha2EncryptedData in the
        //! m.secret_storage.v1.aes-hmac-sha2 section.
        std::map<std::string, AesHmacSha2EncryptedData> encrypted;
};

void
to_json(nlohmann::json &obj, const Secret &secret);

void
from_json(const nlohmann::json &obj, Secret &secret);

struct PBKDF2
{
        //! Required. Must be m.pbkdf2
        std::string algorithm;
        //! Required. The salt used in PBKDF2.
        std::string salt;
        //! Required. The number of iterations to use in PBKDF2.
        uint32_t iterations;
        //! Optional. The number of bits to generate for the key. Defaults to 256.
        uint32_t bits = 256;
};

void
to_json(nlohmann::json &obj, const PBKDF2 &desc);

void
from_json(const nlohmann::json &obj, PBKDF2 &desc);

struct AesHmacSha2KeyDescription
{
        std::string name;      //! Required. The name of the key.
        std::string algorithm; //! Required. The encryption algorithm to be used for this key.
                               //! Currently, only m.secret_storage.v1.aes-hmac-sha2 is supported.
        std::optional<PBKDF2> passphrase; //! See deriving keys from passphrases section for a
                                          //! description of this property.
        std::string iv;                   //! The 16-byte initialization vector, encoded as base64.
        std::string mac; //! The MAC of the result of encrypting 32 bytes of 0, encoded as base64.

        // userid -> key -> key (undocumented)
        std::map<std::string, std::map<std::string, std::string>> signatures;
};

void
to_json(nlohmann::json &obj, const AesHmacSha2KeyDescription &desc);

void
from_json(const nlohmann::json &obj, AesHmacSha2KeyDescription &desc);

}
}
