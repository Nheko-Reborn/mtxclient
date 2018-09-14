#pragma once

#include <nlohmann/json.hpp>

using json = nlohmann::json;

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
};

//! Deserialization method needed by @p nlohmann::json.
void
from_json(const json &obj, Aliases &content);

//! Serialization method needed by @p nlohmann::json.
void
to_json(json &obj, const Aliases &content);

} // namespace state
} // namespace events
} // namespace mtx
