#include "mtx/events/topic.hpp"

#include <nlohmann/json.hpp>

using json = nlohmann::json;

namespace mtx {
namespace events {
namespace state {

void
from_json(const json &obj, Topic &event)
{
    if (obj.find("topic") != obj.end() && !obj.at("topic").is_null())
        event.topic = obj.at("topic").get<std::string>();
}

void
to_json(json &obj, const Topic &event)
{
    obj["topic"] = event.topic;
}

} // namespace state
} // namespace events
} // namespace mtx
