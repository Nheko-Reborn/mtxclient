#pragma once

#include <nlohmann/json.hpp>

#include <string>

namespace mtx {
namespace events {
namespace msg {

//! Content for the `m.room.redaction` state event.
struct Redaction
{
        //! The reason for the redaction, if any.
        std::string reason;
};

void
from_json(const nlohmann::json &obj, Redaction &event);

void
to_json(nlohmann::json &obj, const Redaction &event);

//! Stripped out contente for redacted events.
struct Redacted
{};

inline void
from_json(const nlohmann::json &, Redacted &)
{}

inline void
to_json(nlohmann::json &, const Redacted &)
{}

} // namespace msg
} // namespace events
} // namespace mtx
