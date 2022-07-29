#pragma once

/// @file
/// @brief Events describing redactions and redacted content.

#if __has_include(<nlohmann/json_fwd.hpp>)
#include <nlohmann/json_fwd.hpp>
#else
#include <nlohmann/json.hpp>
#endif

#include <string>

namespace mtx {
namespace events {
namespace msg {

//! Content for the `m.room.redaction` state event.
struct Redaction
{
    //! The reason for the redaction, if any.
    std::string reason;

    friend void from_json(const nlohmann::json &obj, Redaction &event);
    friend void to_json(nlohmann::json &obj, const Redaction &event);
};

//! Stripped out content for redacted events.
struct Redacted
{
    friend inline void from_json(const nlohmann::json &, Redacted &) {}
    friend inline void to_json(nlohmann::json &, const Redacted &) {}
};

} // namespace msg
} // namespace events
} // namespace mtx
