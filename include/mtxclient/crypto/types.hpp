#pragma once

#include "mtxclient/utils.hpp"
#include <nlohmann/json.hpp>

STRONG_TYPE(UserId, std::string)
STRONG_TYPE(DeviceId, std::string)
STRONG_TYPE(RoomId, std::string)

namespace mtx {
namespace crypto {

constexpr auto ED25519           = "ed25519";
constexpr auto CURVE25519        = "curve25519";
constexpr auto SIGNED_CURVE25519 = "signed_curve25519";
constexpr auto MEGOLM_ALGO       = "m.megolm.v1.aes-sha2";

struct ExportedSession
{
        std::map<std::string, std::string> sender_claimed_keys;   // currently unused.
        std::vector<std::string> forwarding_curve25519_key_chain; // currently unused.

        std::string algorithm = MEGOLM_ALGO;
        std::string room_id;
        std::string sender_key;
        std::string session_id;
        std::string session_key;
};

struct ExportedSessionKeys
{
        std::vector<ExportedSession> sessions;
};

struct IdentityKeys
{
        std::string curve25519;
        std::string ed25519;
};

struct OneTimeKeys
{
        using KeyId      = std::string;
        using EncodedKey = std::string;

        std::map<KeyId, EncodedKey> curve25519;
};

void
to_json(nlohmann::json &obj, const ExportedSession &s);

void
from_json(const nlohmann::json &obj, ExportedSession &s);

void
to_json(nlohmann::json &obj, const ExportedSessionKeys &keys);

void
from_json(const nlohmann::json &obj, ExportedSessionKeys &keys);

void
to_json(nlohmann::json &obj, const IdentityKeys &keys);

void
from_json(const nlohmann::json &obj, IdentityKeys &keys);

void
to_json(nlohmann::json &obj, const OneTimeKeys &keys);

void
from_json(const nlohmann::json &obj, OneTimeKeys &keys);

} // namespace crypto
} // namespace mtx
