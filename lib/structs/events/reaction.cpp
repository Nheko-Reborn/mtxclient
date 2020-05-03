#include <nlohmann/json.hpp>
#include <string>

#include "mtx/events/reaction.hpp"

using json = nlohmann::json;

namespace mtx {
namespace events {
namespace msg {

void
from_json(const json &obj, Reaction &event)
{
        if (obj.count("m.relates_to") != 0)
                event.relates_to = obj.at("m.relates_to").get<common::ReactionRelatesTo>();
}

void
to_json(json &obj, const Reaction &event)
{
        obj["m.relates_to"] = event.relates_to;
}

} // namespace msg
} // namespace events
} // namespace mtx
