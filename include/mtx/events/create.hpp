#pragma once

#include <nlohmann/json.hpp>

using json = nlohmann::json;

namespace mtx {
namespace events {
namespace state {

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
};

void
from_json(const json &obj, Create &create);

void
to_json(json &obj, const Create &create);

} // namespace state
} // namespace events
} // namespace mtx
