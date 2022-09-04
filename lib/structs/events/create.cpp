#include "mtx/events/create.hpp"

#include <string>

#include <nlohmann/json.hpp>

using json = nlohmann::json;

namespace mtx {
namespace events {
namespace state {

void
from_json(const json &obj, PreviousRoom &predecessor)
{
    predecessor.room_id  = obj.at("room_id").get<std::string>();
    predecessor.event_id = obj.at("event_id").get<std::string>();
}
void
to_json(json &obj, const PreviousRoom &predecessor)
{
    obj["room_id"]  = predecessor.room_id;
    obj["event_id"] = predecessor.event_id;
}

void
from_json(const json &obj, Create &create)
{
    create.creator = obj.at("creator").get<std::string>();

    if (obj.contains("type") && obj.at("type").is_string())
        create.type = obj.at("type").get<std::string>();

    if (obj.find("m.federate") != obj.end())
        create.federate = obj.at("m.federate").get<bool>();

    // Assume room verison 1 for events where it's not specified
    if (obj.find("room_version") != obj.end())
        create.room_version = obj.at("room_version").get<std::string>();
    else
        create.room_version = "1";

    if (obj.find("predecessor") != obj.end())
        create.predecessor = obj.at("predecessor").get<PreviousRoom>();
}

void
to_json(json &obj, const Create &create)
{
    if (!create.creator.empty())
        obj["creator"] = create.creator;
    obj["m.federate"] = create.federate;
    if (!create.room_version.empty())
        obj["room_version"] = create.room_version;

    if (create.type)
        obj["type"] = create.type.value();
    if (create.predecessor)
        obj["predecessor"] = *create.predecessor;
}

} // namespace state
} // namespace events
} // namespace mtx
