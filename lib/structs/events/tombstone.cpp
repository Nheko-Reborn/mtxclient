#include <nlohmann/json.hpp>

using json = nlohmann::json;

#include "mtx/events/tombstone.hpp"

namespace mtx {
namespace events {
namespace state {

void
from_json(const json &obj, Tombstone &content)
{
    content.body             = obj.value("body", "");
    content.replacement_room = obj.value("replacement_room", "");
}

void
to_json(json &obj, const Tombstone &content)
{
    obj["body"]             = content.body;
    obj["replacement_room"] = content.replacement_room;
}

} // namespace state
} // namespace events
} // namespace mtx
