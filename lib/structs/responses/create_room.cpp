#include <string>

#include "mtx/identifiers.hpp"
#include "mtx/responses/create_room.hpp"

using json = nlohmann::json;

namespace mtx {
namespace responses {

void
from_json(const json &obj, CreateRoom &response)
{
        using namespace mtx::identifiers;
        response.room_id = obj.at("room_id").get<Room>();
}
}
}
