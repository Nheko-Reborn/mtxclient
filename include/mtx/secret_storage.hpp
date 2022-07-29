#pragma once

/// @file
/// @brief Header with SSSS related types.

#include <cstdint>
#include <map>
#include <optional>
#include <string>

#if __has_include(<nlohmann/json_fwd.hpp>)
#include <nlohmann/json_fwd.hpp>
#else
#include <nlohmann/json.hpp>
#endif

namespace mtx {
//! SSSS related types to store encrypted data on the server.
namespace secret_storage {
//! Names of secrets used in the spec.
namespace secrets {
//! Decryption key for online key backup.
constexpr const char megolm_backup_v1[] = "m.megolm_backup.v1";
//! Key to sign own devices
constexpr const char cross_signing_self_signing[] = "m.cross_signing.self_signing";
//! Key to sign other users
constexpr const char cross_signing_user_signing[] = "m.cross_signing.user_signing";
//! Key to sign your own keys like user and self signing keys.
constexpr const char cross_signing_master[] = "m.cross_signing.master";
}

//! A aes-hmac-sha2 encrypted secret.
struct AesHmacSha2EncryptedData
{
    std::string iv;         //!< Required. The 16-byte initialization vector, encoded as base64.
    std::string ciphertext; //!< Required. The AES-CTR-encrypted data, encoded as base64.
    std::string mac;        //!< Required. The MAC, encoded as base64.

    friend void to_json(nlohmann::json &obj, const AesHmacSha2EncryptedData &data);
    friend void from_json(const nlohmann::json &obj, AesHmacSha2EncryptedData &data);
};

//! A secret, encrypted with one or more algorithms.
struct Secret
{
    /// @brief Required. Map from key ID the encrypted data.
    ///
    /// The exact format for the encrypted data is dependent on the key algorithm. See the
    /// definition of AesHmacSha2EncryptedData in the m.secret_storage.v1.aes-hmac-sha2 section.
    std::map<std::string, AesHmacSha2EncryptedData> encrypted;

    friend void to_json(nlohmann::json &obj, const Secret &secret);
    friend void from_json(const nlohmann::json &obj, Secret &secret);
};

//! Information about the key derivation from a passphrase.
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

    friend void to_json(nlohmann::json &obj, const PBKDF2 &desc);
    friend void from_json(const nlohmann::json &obj, PBKDF2 &desc);
};

//! Description of the key for a secret.
struct AesHmacSha2KeyDescription
{
    std::string name; //!< Required. The name of the key.
    /// @brief Required. The encryption algorithm to be used for this key.
    /// Currently, only m.secret_storage.v1.aes-hmac-sha2 is supported.
    std::string algorithm;
    //! See deriving keys from passphrases section for a description of this property.
    std::optional<PBKDF2> passphrase;
    std::string iv;  //!< The 16-byte initialization vector, encoded as base64.
    std::string mac; //!< The MAC of the result of encrypting 32 bytes of 0, encoded as base64.

    //! userid -> key -> key (undocumented)
    std::map<std::string, std::map<std::string, std::string>> signatures;

    friend void to_json(nlohmann::json &obj, const AesHmacSha2KeyDescription &desc);
    friend void from_json(const nlohmann::json &obj, AesHmacSha2KeyDescription &desc);
};
}
}
