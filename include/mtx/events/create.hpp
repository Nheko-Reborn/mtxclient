#pragma once

/// @file
/// @brief The first event in a room.

#include <optional>

#if __has_include(<nlohmann/json_fwd.hpp>)
#include <nlohmann/json_fwd.hpp>
#else
#include <nlohmann/json.hpp>
#endif

namespace mtx {
namespace events {
namespace state {
//! The predecessor of this room.
struct PreviousRoom
{
    //! Required. The ID of the old room.
    std::string room_id;
    //! Required. The event ID of the last known event in the old room.
    std::string event_id;
};

//! Definitions of different room types.
namespace room_type {
//! The room type for a space.
constexpr std::string_view space = "m.space";
//! MSC for policy list rooms, see https://github.com/matrix-org/matrix-spec-proposals/pull/3784
constexpr std::string_view exp_policy = "support.feline.policy.lists.msc.v1";
}

//! Content of the `m.room.create` event.
//
//! This is the first event in a room and cannot be changed.
//! It acts as the root of all other events.
struct Create
{
    //! The `user_id` of the room creator. This is set by the homeserver.
    std::string creator;

    //! The room type, for example `m.space` for spaces.
    std::optional<std::string> type;

    //! Whether users on other servers can join this room.
    //! Defaults to **true** if key does not exist.
    bool federate = true;

    //! The version of the room. Defaults to "1" if the key does not exist.
    std::string room_version = "1";

    //! A reference to the room this room replaces, if the previous room was upgraded.
    std::optional<PreviousRoom> predecessor;

    friend void from_json(const nlohmann::json &obj, Create &create);

    friend void to_json(nlohmann::json &obj, const Create &create);
};

} // namespace state
} // namespace events
} // namespace mtx
