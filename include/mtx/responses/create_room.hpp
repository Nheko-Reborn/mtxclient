#pragma once

#include <nlohmann/json.hpp>

namespace mtx {
namespace responses {
//! Response from the `POST /_matrix/client/r0/createRoom` endpoint.
struct CreateRoom
{
        //! The room ID of the newly created room.
        mtx::identifiers::Room room_id;
};

void
from_json(const nlohmann::json &obj, CreateRoom &response);
}
}
