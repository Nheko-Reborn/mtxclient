#include <nlohmann/json.hpp>

#include "mtx/identifiers.hpp"
#include "mtx/responses/create_room.hpp"

namespace mtx {
namespace responses {

void
from_json(const nlohmann::json &obj, CreateRoom &response)
{
    response.room_id = obj.at("room_id").get<identifiers::Room>();
}
}
}
