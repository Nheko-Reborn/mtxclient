#pragma once

/// @file
/// @brief Various type definitions for the crypto API.

#if __has_include(<nlohmann/json_fwd.hpp>)
#include <nlohmann/json_fwd.hpp>
#else
#include <nlohmann/json.hpp>
#endif

namespace mtx {
namespace crypto {
//! Constant for ed25519 keys
constexpr auto ED25519 = "ed25519";
//! Constant for curve25519 keys
constexpr auto CURVE25519 = "curve25519";
//! Constant for signed curve25519 keys
constexpr auto SIGNED_CURVE25519 = "signed_curve25519";
//! The algorithm used for group messages.
constexpr auto MEGOLM_ALGO = "m.megolm.v1.aes-sha2";

//! An exported megolm group session
struct ExportedSession
{
    //! Required. The Ed25519 key of the device which initiated the session originally.
    std::map<std::string, std::string> sender_claimed_keys; // currently unused.
    //! Required. Chain of Curve25519 keys through which this session was forwarded, via
    //! m.forwarded_room_key events.
    std::vector<std::string> forwarding_curve25519_key_chain; // currently unused.

    //! Required. The encryption algorithm that the session uses. Must be m.megolm.v1.aes-sha2.
    std::string algorithm = MEGOLM_ALGO;
    //! Required. The room where the session is used.
    std::string room_id;
    //! Required. The Curve25519 key of the device which initiated the session originally.
    std::string sender_key;
    //! Required. The ID of the session.
    std::string session_id;
    //! Required. The key for the session.
    std::string session_key;

    friend void to_json(nlohmann::json &obj, const ExportedSession &s);
    friend void from_json(const nlohmann::json &obj, ExportedSession &s);
};

//! A list of exported sessions.
struct ExportedSessionKeys
{
    //! The actual sessions.
    std::vector<ExportedSession> sessions;

    friend void to_json(nlohmann::json &obj, const ExportedSessionKeys &keys);
    friend void from_json(const nlohmann::json &obj, ExportedSessionKeys &keys);
};

//! A pair of keys connected to an olm account.
struct IdentityKeys
{
    //! The identity key.
    std::string curve25519;
    //! The signing key.
    std::string ed25519;

    friend void to_json(nlohmann::json &obj, const IdentityKeys &keys);
    friend void from_json(const nlohmann::json &obj, IdentityKeys &keys);
};

//! A list of one time keys.
struct OneTimeKeys
{
    //! The key id type.
    using KeyId = std::string;
    //! The type for the keys.
    using EncodedKey = std::string;

    //! The one time keys by key id.
    std::map<KeyId, EncodedKey> curve25519;

    friend void to_json(nlohmann::json &obj, const OneTimeKeys &keys);
    friend void from_json(const nlohmann::json &obj, OneTimeKeys &keys);
};

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

    [[nodiscard]] T &get() { return value_; }
    [[nodiscard]] T const &get() const { return value_; }

private:
    T value_;
};

// Macro for concisely defining a strong type
#define STRONG_TYPE(type_name, value_type)                                                         \
    struct type_name : mtx::crypto::strong_type<value_type, type_name>                             \
    {                                                                                              \
        using strong_type::strong_type;                                                            \
    };

} // namespace crypto
} // namespace mtx

STRONG_TYPE(UserId, std::string)
STRONG_TYPE(DeviceId, std::string)
STRONG_TYPE(RoomId, std::string)
