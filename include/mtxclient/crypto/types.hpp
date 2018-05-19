#pragma once

#include "mtxclient/utils.hpp"
#include <json.hpp>

STRONG_TYPE(UserId, std::string)
STRONG_TYPE(DeviceId, std::string)
STRONG_TYPE(RoomId, std::string)

namespace mtx {
namespace crypto {

static constexpr const char *ED25519    = "ed25519";
static constexpr const char *CURVE25519 = "curve25519";

struct IdentityKeys
{
        std::string curve25519;
        std::string ed25519;
};

inline void
to_json(nlohmann::json &obj, const IdentityKeys &keys)
{
        obj[ED25519]    = keys.ed25519;
        obj[CURVE25519] = keys.curve25519;
}

inline void
from_json(const nlohmann::json &obj, IdentityKeys &keys)
{
        keys.ed25519    = obj.at(ED25519).get<std::string>();
        keys.curve25519 = obj.at(CURVE25519).get<std::string>();
}

struct OneTimeKeys
{
        using KeyId      = std::string;
        using EncodedKey = std::string;

        std::map<KeyId, EncodedKey> curve25519;
};

inline void
to_json(nlohmann::json &obj, const OneTimeKeys &keys)
{
        obj["curve25519"] = keys.curve25519;
}

inline void
from_json(const nlohmann::json &obj, OneTimeKeys &keys)
{
        keys.curve25519 = obj.at("curve25519").get<std::map<std::string, std::string>>();
}
}
};
