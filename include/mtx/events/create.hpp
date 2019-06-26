#pragma once

#include <boost/optional.hpp>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

namespace mtx {
namespace events {
namespace state {

struct PreviousRoom
{
        //! Required. The ID of the old room.
        std::string room_id;
        //! Required. The event ID of the last known event in the old room.
        std::string event_id;
};

//! Content of the `m.room.create` event.
//
//! This is the first event in a room and cannot be changed.
//! It acts as the root of all other events.
struct Create
{
        //! The `user_id` of the room creator. This is set by the homeserver.
        std::string creator;

        //! Whether users on other servers can join this room.
        //! Defaults to **true** if key does not exist.
        bool federate = true;

        //! The version of the room. Defaults to "1" if the key does not exist.
        std::string room_version = "1";

        //! A reference to the room this room replaces, if the previous room was upgraded.
        boost::optional<PreviousRoom> predecessor;
};

void
from_json(const json &obj, Create &create);

void
to_json(json &obj, const Create &create);

} // namespace state
} // namespace events
} // namespace mtx
