#pragma once

/// @file
/// @brief Response from creating a room.

#if __has_include(<nlohmann/json_fwd.hpp>)
#include <nlohmann/json_fwd.hpp>
#else
#include <nlohmann/json.hpp>
#endif

#include <mtx/identifiers.hpp>

namespace mtx {
namespace responses {
//! Response from the `POST /_matrix/client/r0/createRoom` endpoint.
struct CreateRoom
{
    //! The room ID of the newly created room.
    mtx::identifiers::Room room_id;

    friend void from_json(const nlohmann::json &obj, CreateRoom &response);
};
}
}
