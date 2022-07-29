#pragma once

#if __has_include(<nlohmann/json_fwd.hpp>)
#include <nlohmann/json_fwd.hpp>
#else
#include <nlohmann/json.hpp>
#endif

#include <map>
#include <string>
#include <vector>

/// @file
/// @brief Common types used by the crypto related endpoints. Common types used by room directory
/// endpoints.

namespace mtx {
namespace crypto {

using AlgorithmDevice = std::string;

struct UnsignedDeviceInfo
{
    //! The display name which the user set on the device.
    std::string device_display_name;

    friend void from_json(const nlohmann::json &obj, UnsignedDeviceInfo &res);
    friend void to_json(nlohmann::json &obj, const UnsignedDeviceInfo &res);
};

struct DeviceKeys
{
    //! The ID of the user the device belongs to.
    std::string user_id;
    //! The ID of the device these keys belong to.
    std::string device_id;
    //! The encryption algorithms supported by this device.
    std::vector<std::string> algorithms = {"m.olm.v1.curve25519-aes-sha2", "m.megolm.v1.aes-sha2"};
    //! Public identity keys.
    //! The names of the properties should be in the format <algorithm>:<device_id>.
    //! The keys themselves should be encoded as specified by the key algorithm.
    std::map<AlgorithmDevice, std::string> keys;
    //! Signatures for the device key object.
    //! A map from user ID, to a map from <algorithm>:<device_id> to the signature.
    std::map<std::string, std::map<AlgorithmDevice, std::string>> signatures;
    ///! Additional data added to the device key information
    //! by intermediate servers, and not covered by the signatures.
    UnsignedDeviceInfo unsigned_info;

    friend void from_json(const nlohmann::json &obj, DeviceKeys &res);
    friend void to_json(nlohmann::json &obj, const DeviceKeys &res);
};

struct CrossSigningKeys
{
    //! The ID of the user the device belongs to.
    std::string user_id;
    //! mentions the purpose of the key like either master,user_signing,self_signing
    std::vector<std::string> usage;
    //! Public keys.
    //! The names of the properties should be in the format <algorithm>:<public_key>.
    std::map<std::string, std::string> keys;
    //! Signatures for the cross signing key object.
    //! A map from user ID, to a map from <algorithm>:<public_key> to the signature.
    std::map<std::string, std::map<std::string, std::string>> signatures;

    friend void from_json(const nlohmann::json &obj, CrossSigningKeys &res);
    friend void to_json(nlohmann::json &obj, const CrossSigningKeys &res);
};

struct JWK
{
    //! Required. Key type. Must be oct.
    std::string kty;
    //! Required. Key operations. Must at least contain encrypt and decrypt.
    std::vector<std::string> key_ops;
    //! Required. Algorithm. Must be A256CTR.
    std::string alg;
    //! Required. The key, encoded as urlsafe unpadded base64.
    std::string k;
    //! Required. Extractable. Must be true. This is a W3C extension.
    bool ext;

    friend void from_json(const nlohmann::json &obj, JWK &res);
    friend void to_json(nlohmann::json &obj, const JWK &res);
};

struct EncryptedFile
{
    //! Required. The URL to the file.
    std::string url;
    //! Required. A JSON Web Key object. (The encryption key)
    JWK key;
    //! Required. The Initialisation Vector used by AES-CTR, encoded as unpadded base64.
    std::string iv;
    //! Required. A map from an algorithm name to a hash of the ciphertext, encoded as unpadded
    //! base64. Clients should support the SHA-256 hash, which uses the key sha256.
    std::map<std::string, std::string> hashes;
    //! Required. Version of the encrypted attachments protocol. Must be v2.
    std::string v;

    friend void from_json(const nlohmann::json &obj, EncryptedFile &res);
    friend void to_json(nlohmann::json &obj, const EncryptedFile &res);
};

} // namespace crypto

//
namespace common {
//! Whether or not the room will be visible by non members.
enum class RoomVisibility
{
    //! A private visibility will hide the room from the published room list.
    Private,
    //! Indicates that the room will be shown in the published room list.
    Public,
};

inline std::string
visibilityToString(RoomVisibility visibility)
{
    if (visibility == RoomVisibility::Private) {
        return "private";
    }

    return "public";
}

inline RoomVisibility
stringToVisibility(const std::string &s)
{
    if (s == "private") {
        return RoomVisibility::Private;
    }
    return RoomVisibility::Public;
}
} // namespace common
} // namespace mtx
