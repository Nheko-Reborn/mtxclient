#pragma once

/// @file
/// @brief E2EE related endpoints.

#if __has_include(<nlohmann/json_fwd.hpp>)
#include <nlohmann/json_fwd.hpp>
#else
#include <nlohmann/json.hpp>
#endif

#include "mtx/common.hpp"
#include "mtx/lightweight_error.hpp"

#include <map>
#include <string>

namespace mtx {
namespace responses {
//! Response from the `POST /_matrix/client/r0/keys/upload` endpoint.
struct UploadKeys
{
    //! For each key algorithm, the number of unclaimed one-time keys
    //! of that type currently held on the server for this device.
    std::map<std::string, uint32_t> one_time_key_counts;

    friend void from_json(const nlohmann::json &obj, UploadKeys &response);
};

using DeviceToKeysMap = std::map<std::string, mtx::crypto::DeviceKeys>;

//! Response from the `POST /_matrix/client/r0/keys/query` endpoint.
struct QueryKeys
{
    //! If any remote homeservers could not be reached, they are
    //! recorded here. The names of the properties are the names
    //! of the unreachable servers.
    std::map<std::string, nlohmann::json> failures;
    //! Information on the queried devices.
    //! A map from user ID, to a map from device ID to device information.
    //! For each device, the information returned will be the same
    //! as uploaded via /keys/upload, with the addition of an unsigned property
    std::map<std::string, DeviceToKeysMap> device_keys;
    //! A map from user ID, to information about master_keys.
    std::map<std::string, mtx::crypto::CrossSigningKeys> master_keys;
    //! A map from user ID, to information about user_signing_keys.
    std::map<std::string, mtx::crypto::CrossSigningKeys> user_signing_keys;
    //! A map from user ID, to information about self_signing_keys.
    std::map<std::string, mtx::crypto::CrossSigningKeys> self_signing_keys;

    friend void to_json(nlohmann::json &obj, const QueryKeys &response);
    friend void from_json(const nlohmann::json &obj, QueryKeys &response);
};

//! Request for `POST /_matrix/client/r0/keys/upload`.
struct KeySignaturesUpload
{
    //! Errors returned during upload.
    std::map<std::string, std::map<std::string, mtx::errors::LightweightError>> errors;

    friend void from_json(const nlohmann::json &obj, KeySignaturesUpload &response);
};

//! Response from the `POST /_matrix/client/r0/keys/claim` endpoint.
struct ClaimKeys
{
    //! If any remote homeservers could not be reached, they are
    //! recorded here. The names of the properties are the names
    //! of the unreachable servers.
    std::map<std::string, nlohmann::json> failures;
    //! One-time keys for the queried devices. A map from user ID,
    //! to a map from <algorithm>:<key_id> to the key object.
    std::map<std::string, std::map<std::string, nlohmann::json>> one_time_keys;

    friend void from_json(const nlohmann::json &obj, ClaimKeys &response);
};

//! Response from the `GET /_matrix/client/r0/keys/changes` endpoint.
struct KeyChanges
{
    //! The Matrix User IDs of all users who updated their device identity keys.
    std::vector<std::string> changed;
    //! The Matrix User IDs of all users who may have left all the end-to-end
    //! encrypted rooms they previously shared with the user.
    std::vector<std::string> left;

    friend void from_json(const nlohmann::json &obj, KeyChanges &response);
};

//! KeysBackup related responses.
namespace backup {
//! Encrypted session data using the m.megolm_backup.v1.curve25519-aes-sha2 algorithm
struct EncryptedSessionData
{
    //! Generate an ephemeral curve25519 key, and perform an ECDH with the ephemeral key and the
    //! backup's public key to generate a shared secret. The public half of the ephemeral key,
    //! encoded using unpadded base64, becomes the ephemeral property
    std::string ephemeral;
    //! Stringify the JSON object, and encrypt it using AES-CBC-256 with PKCS#7 padding. This
    //! encrypted data, encoded using unpadded base64, becomes the ciphertext property of the
    //! session_data.
    std::string ciphertext;
    //! Pass the raw encrypted data (prior to base64 encoding) through HMAC-SHA-256 using the
    //! MAC key generated above. The first 8 bytes of the resulting MAC are base64-encoded, and
    //! become the mac property of the session_data.
    std::string mac;

    friend void from_json(const nlohmann::json &obj, EncryptedSessionData &response);
    friend void to_json(nlohmann::json &obj, const EncryptedSessionData &response);
};

//! Responses from the `GET /_matrix/client/r0/room_keys/keys/{room_id}/{session_id}` endpoint
struct SessionBackup
{
    //! Required. The index of the first message in the session that the key can decrypt.
    int64_t first_message_index;
    //! Required. The number of times this key has been forwarded via key-sharing between
    //! devices.
    int64_t forwarded_count;
    //! Required. Whether the device backing up the key verified the device that the key is
    //! from.
    bool is_verified;
    //! Required. Algorithm-dependent data. See the documentation for the backup algorithms in
    //! Server-side key backups for more information on the expected format of the data.
    EncryptedSessionData session_data;

    friend void from_json(const nlohmann::json &obj, SessionBackup &response);
    friend void to_json(nlohmann::json &obj, const SessionBackup &response);
};

//! Responses from the `GET /_matrix/client/r0/room_keys/keys/{room_id}` endpoint
struct RoomKeysBackup
{
    //! map of session id to the individual sessions
    std::map<std::string, SessionBackup> sessions;

    friend void from_json(const nlohmann::json &obj, RoomKeysBackup &response);
    friend void to_json(nlohmann::json &obj, const RoomKeysBackup &response);
};

//! Responses from the `GET /_matrix/client/r0/room_keys/keys` endpoint
struct KeysBackup
{
    //! map of room id to map of session ids to backups of individual sessions
    std::map<std::string, RoomKeysBackup> rooms;

    friend void from_json(const nlohmann::json &obj, KeysBackup &response);
    friend void to_json(nlohmann::json &obj, const KeysBackup &response);
};

constexpr const char *megolm_backup_v1 = "m.megolm_backup.v1.curve25519-aes-sha2";
//! Responses from the `GET /_matrix/client/r0/room_keys/version` endpoint
struct BackupVersion
{
    //! Required. The algorithm used for storing backups. Must be
    //! 'm.megolm_backup.v1.curve25519-aes-sha2'.
    std::string algorithm;
    //! Required. Algorithm-dependent data. See the documentation for the backup algorithms in
    //! Server-side key backups for more information on the expected format of the data.
    std::string auth_data;
    //! Required. The number of keys stored in the backup.
    int64_t count;
    //! Required. An opaque string representing stored keys in the backup. Clients can
    //! compare it with the etag value they received in the request of their last key storage
    //! request. If not equal, another client has modified the backup
    std::string etag;
    //! Required. The backup version
    std::string version;

    friend void from_json(const nlohmann::json &obj, BackupVersion &response);
    friend void to_json(nlohmann::json &obj, const BackupVersion &response);
};

//! The SessionData stored in the KeysBackup.
struct SessionData
{
    //! Required. The end-to-end message encryption algorithm that the key is
    // for. Must be m.megolm.v1.aes-sha2.
    std::string algorithm;
    // Required. Chain of Curve25519 keys through which this
    // session was forwarded, via m.forwarded_room_key events.
    std::vector<std::string> forwarding_curve25519_key_chain;
    // Required. Unpadded base64-encoded device curve25519 key.
    std::string sender_key;
    // Required. A map from algorithm name (ed25519) to the identity
    // key for the sending device.
    std::map<std::string, std::string> sender_claimed_keys;
    // Required. Unpadded base64-encoded session key in session-sharing format.
    std::string session_key;

    friend void to_json(nlohmann::json &obj, const SessionData &desc);
    friend void from_json(const nlohmann::json &obj, SessionData &desc);
};
}
} // namespace responses
} // namespace mtx
