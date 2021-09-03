#include "mtxclient/crypto/types.hpp"

#include <nlohmann/json.hpp>

namespace mtx {
namespace crypto {

void
to_json(nlohmann::json &obj, const ExportedSession &s)
{
    obj["sender_claimed_keys"]             = s.sender_claimed_keys;
    obj["forwarding_curve25519_key_chain"] = s.forwarding_curve25519_key_chain;

    obj["algorithm"]   = s.algorithm;
    obj["room_id"]     = s.room_id;
    obj["sender_key"]  = s.sender_key;
    obj["session_id"]  = s.session_id;
    obj["session_key"] = s.session_key;
}

void
from_json(const nlohmann::json &obj, ExportedSession &s)
{
    s.room_id     = obj.at("room_id").get<std::string>();
    s.sender_key  = obj.at("sender_key").get<std::string>();
    s.session_id  = obj.at("session_id").get<std::string>();
    s.session_key = obj.at("session_key").get<std::string>();

    using ClaimedKeys = std::map<std::string, std::string>;
    using KeyChain    = std::vector<std::string>;

    if (obj.find("sender_claimed_keys") != obj.end())
        s.sender_claimed_keys = obj.at("sender_claimed_keys").get<ClaimedKeys>();

    if (obj.find("forwarding_curve25519_key_chain") != obj.end())
        s.forwarding_curve25519_key_chain =
          obj.at("forwarding_curve25519_key_chain").get<KeyChain>();
}

void
to_json(nlohmann::json &obj, const ExportedSessionKeys &keys)
{
    obj = keys.sessions;
}

void
from_json(const nlohmann::json &obj, ExportedSessionKeys &keys)
{
    try {
        keys.sessions = obj.get<std::vector<ExportedSession>>();
        // might be the old format.
    } catch (const nlohmann::json::exception &) {
        keys.sessions = obj.at("sessions").get<std::vector<ExportedSession>>();
    }
}

void
to_json(nlohmann::json &obj, const IdentityKeys &keys)
{
    obj[ED25519]    = keys.ed25519;
    obj[CURVE25519] = keys.curve25519;
}

void
from_json(const nlohmann::json &obj, IdentityKeys &keys)
{
    keys.ed25519    = obj.at(ED25519).get<std::string>();
    keys.curve25519 = obj.at(CURVE25519).get<std::string>();
}

void
to_json(nlohmann::json &obj, const OneTimeKeys &keys)
{
    obj[CURVE25519] = keys.curve25519;
}

void
from_json(const nlohmann::json &obj, OneTimeKeys &keys)
{
    keys.curve25519 = obj.at(CURVE25519).get<std::map<std::string, std::string>>();
}

} // namespace crypto
} // namespace mtx
