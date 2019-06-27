#include <string>

#include "mtx/events/create.hpp"

using json = nlohmann::json;

namespace mtx {
namespace events {
namespace state {

void
from_json(const json &obj, PreviousRoom &predecessor)
{
        predecessor.room_id  = obj.at("room_id");
        predecessor.event_id = obj.at("event_id");
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
        create.creator = obj.at("creator");

        if (obj.find("m.federate") != obj.end())
                create.federate = obj.at("m.federate").get<bool>();

        if (obj.find("room_version") != obj.end())
                create.room_version = obj.at("room_version");

        if (obj.find("predecessor") != obj.end())
                create.predecessor = obj.at("predecessor").get<PreviousRoom>();
}

void
to_json(json &obj, const Create &create)
{
        obj["creator"]      = create.creator;
        obj["m.federate"]   = create.federate;
        obj["room_version"] = create.room_version;

        if (create.predecessor)
                obj["predecessor"] = *create.predecessor;
}

} // namespace state
} // namespace events
} // namespace mtx
