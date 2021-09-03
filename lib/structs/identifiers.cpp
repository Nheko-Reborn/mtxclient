#include "mtx/identifiers.hpp"

#include <nlohmann/json.hpp>

namespace mtx {
namespace identifiers {

void
from_json(const nlohmann::json &obj, User &user)
{
    user = parse<User>(obj.get<std::string>());
}

void
to_json(nlohmann::json &obj, const User &user)
{
    obj = user.to_string();
}

void
from_json(const nlohmann::json &obj, Room &room)

{
    room = parse<Room>(obj.get<std::string>());
}

void
to_json(nlohmann::json &obj, const Room &room)
{
    obj = room.to_string();
}

void
from_json(const nlohmann::json &obj, Event &event)
{
    event = parse<Event>(obj.get<std::string>());
}

void
to_json(nlohmann::json &obj, const Event &event)
{
    obj = event.to_string();
}
}
}
