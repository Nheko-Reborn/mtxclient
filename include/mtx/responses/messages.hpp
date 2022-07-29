#pragma once

/// @file
/// @brief Reponses from the message pagination API.

#include <string>

#if __has_include(<nlohmann/json_fwd.hpp>)
#include <nlohmann/json_fwd.hpp>
#else
#include <nlohmann/json.hpp>
#endif

#include "mtx/events/collections.hpp"

namespace mtx {
namespace responses {

//! Response of the `GET /_matrix/client/r0/rooms/{roomId}/messages` endpoint.
//
//! This API returns a list of message and state events for a room.
//! It uses pagination query parameters to paginate history in the room.
struct Messages
{
    //! The token the pagination starts from.
    std::string start;
    //! The token the pagination ends at.
    std::string end;
    //! A list of room events.
    std::vector<mtx::events::collections::TimelineEvents> chunk;

    friend void from_json(const nlohmann::json &obj, Messages &messages);
};
}
}
