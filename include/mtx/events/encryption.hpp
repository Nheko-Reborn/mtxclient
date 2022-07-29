#pragma once

/// @file
/// @brief The event enabling encryption in a room.

#if __has_include(<nlohmann/json_fwd.hpp>)
#include <nlohmann/json_fwd.hpp>
#else
#include <nlohmann/json.hpp>
#endif

namespace mtx {
namespace events {
namespace state {

//! Content of the `m.room.encryption` event.
//
//! A client can enable encryption to a room by sending this event.
struct Encryption
{
    //! Defines which encryption algorithm should be used for encryption.
    //! Currently only m.megolm.v1-aes-sha2 is permitted.
    std::string algorithm = "m.megolm.v1.aes-sha2";
    //! How long the session should be used before changing it. 604800000 (a week) is the
    //! recommended default.
    uint64_t rotation_period_ms = 604800000;
    //! How many messages should be sent before changing the session. 100 is the recommended
    //! default.
    uint64_t rotation_period_msgs = 100;

    friend void from_json(const nlohmann::json &obj, Encryption &encryption);
    friend void to_json(nlohmann::json &obj, const Encryption &encryption);
};

} // namespace state
} // namespace events
} // namespace mtx
