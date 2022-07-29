#pragma once

/// @file
/// @brief An event describing a room upgrade or shutdown.

#if __has_include(<nlohmann/json_fwd.hpp>)
#include <nlohmann/json_fwd.hpp>
#else
#include <nlohmann/json.hpp>
#endif

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

    //! Deserialization method needed by @p nlohmann::json.
    friend void from_json(const nlohmann::json &obj, Tombstone &content);

    //! Serialization method needed by @p nlohmann::json.
    friend void to_json(nlohmann::json &obj, const Tombstone &content);
};

} // namespace state
} // namespace events
} // namespace mtx
