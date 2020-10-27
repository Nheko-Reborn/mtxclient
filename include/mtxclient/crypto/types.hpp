#pragma once

#if __has_include(<nlohmann/json_fwd.hpp>)
#include <nlohmann/json_fwd.hpp>
#else
#include <nlohmann/json.hpp>
#endif

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

template<class T, class Name>
class strong_type
{
public:
        strong_type() = default;
        explicit strong_type(const T &value)
          : value_(value)
        {}
        explicit strong_type(T &&value)
          : value_(std::forward<T>(value))
        {}

        operator T &() noexcept { return value_; }
        constexpr operator const T &() const noexcept { return value_; }

        T &get() { return value_; }
        T const &get() const { return value_; }

private:
        T value_;
};

// Macro for concisely defining a strong type
#define STRONG_TYPE(type_name, value_type)                                                         \
        struct type_name : mtx::crypto::strong_type<value_type, type_name>                         \
        {                                                                                          \
                using strong_type::strong_type;                                                    \
        };

} // namespace crypto
} // namespace mtx

STRONG_TYPE(UserId, std::string)
STRONG_TYPE(DeviceId, std::string)
STRONG_TYPE(RoomId, std::string)
