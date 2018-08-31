#include <string>

#include "mtx/events/create.hpp"

using json = nlohmann::json;

namespace mtx {
namespace events {
namespace state {

void
from_json(const json &obj, Create &create)
{
        create.creator = obj.at("creator");

        if (obj.find("m.federate") != obj.end())
                create.federate = obj.at("m.federate").get<bool>();
}

void
to_json(json &obj, const Create &create)
{
        obj["creator"]    = create.creator;
        obj["m.federate"] = create.federate;
}

} // namespace state
} // namespace events
} // namespace mtx
