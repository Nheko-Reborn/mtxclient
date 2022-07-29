#pragma once

/// @file
/// @brief The fully read marker for this user.

#include <string>

#if __has_include(<nlohmann/json_fwd.hpp>)
#include <nlohmann/json_fwd.hpp>
#else
#include <nlohmann/json.hpp>
#endif

namespace mtx {
namespace events {
namespace account_data {

/// @brief The fully read marker for this user / `m.fully_read`.
///
/// The history for a given room may be split into three sections: messages the user has read (or
/// indicated they aren't interested in them), messages the user might have read some but not
/// others, and messages the user hasn't seen yet. The "fully read marker" (also known as a "read
/// marker") marks the last event of the first section, whereas the user's read receipt marks the
/// last event of the second section.
struct FullyRead
{
    //! Required. The event the user's read marker is located at in the room.
    std::string event_id;

    //! Deserialization method needed by @p nlohmann::json.
    friend void from_json(const nlohmann::json &obj, FullyRead &content);

    //! Serialization method needed by @p nlohmann::json.
    friend void to_json(nlohmann::json &obj, const FullyRead &content);
};

} // namespace state
} // namespace events
} // namespace mtx
