#pragma once

#include <nlohmann/json.hpp>

using json = nlohmann::json;

namespace mtx {
namespace events {
namespace state {

//! Content for the `m.room.tombstone` event.
//
//! A state event signifying that a room has been
//! upgraded to a different room version, and
//! that clients should go there.
struct Tombstone
{
        //! Required. A server-defined message.
        std::string body;
        //! Required. The new room the client should be visiting.
        std::string replacement_room;
};

//! Deserialization method needed by @p nlohmann::json.
void
from_json(const json &obj, Tombstone &content);

//! Serialization method needed by @p nlohmann::json.
void
to_json(json &obj, const Tombstone &content);

} // namespace state
} // namespace events
} // namespace mtx
