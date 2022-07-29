#pragma once

/// @file
/// @brief Typing notifications.

#include <string>
#include <vector>

#if __has_include(<nlohmann/json_fwd.hpp>)
#include <nlohmann/json_fwd.hpp>
#else
#include <nlohmann/json.hpp>
#endif

namespace mtx {
namespace events {
//! Ephemeral events not part of the timeline like typing and read notifications.
namespace ephemeral {

/// @brief Typing notifications / `m.typing`.
///
/// Users may wish to be informed when another user is typing in a room. This can be achieved using
/// typing notifications. These are ephemeral events scoped to a room_id. This means they do not
/// form part of the Event Graph but still have a room_id key.
struct Typing
{
    //! Required. The list of user IDs typing in this room, if any.
    std::vector<std::string> user_ids;

    //! Deserialization method needed by @p nlohmann::json.
    friend void from_json(const nlohmann::json &obj, Typing &content);

    //! Serialization method needed by @p nlohmann::json.
    friend void to_json(nlohmann::json &obj, const Typing &content);
};

} // namespace state
} // namespace events
} // namespace mtx
