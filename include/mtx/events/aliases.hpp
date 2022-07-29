#pragma once

/// @file
/// @brief Alias events.

#include <string>
#include <vector>

#if __has_include(<nlohmann/json_fwd.hpp>)
#include <nlohmann/json_fwd.hpp>
#else
#include <nlohmann/json.hpp>
#endif

namespace mtx {
namespace events {
namespace state {

//! Content for the `m.room.aliases` event.
//
//! This event is sent by a homeserver directly to inform of changes to
//! the list of aliases it knows about for that room. The `state_key`
//! for this event is set to the homeserver which owns the room alias.
//! The entire set of known aliases for the room is the union of all
//! the `m.room.aliases` events, one for each homeserver.
struct Aliases
{
    //! A list of room aliases.
    std::vector<std::string> aliases;

    //! Deserialization method needed by @p nlohmann::json.
    friend void from_json(const nlohmann::json &obj, Aliases &content);

    //! Serialization method needed by @p nlohmann::json.
    friend void to_json(nlohmann::json &obj, const Aliases &content);
};

} // namespace state
} // namespace events
} // namespace mtx
