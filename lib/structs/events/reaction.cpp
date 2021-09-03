#include <nlohmann/json.hpp>
#include <string>

#include "mtx/events/reaction.hpp"

using json = nlohmann::json;

namespace mtx {
namespace events {
namespace msg {

void
from_json(const json &obj, Reaction &content)
{
    content.relations = common::parse_relations(obj);
}

void
to_json(json &obj, const Reaction &content)
{
    obj = nlohmann::json::object();

    common::apply_relations(obj, content.relations);
}

} // namespace msg
} // namespace events
} // namespace mtx
