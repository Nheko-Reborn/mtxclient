#include <string>

#include "mtx/events/topic.hpp"

using json = nlohmann::json;

namespace mtx {
namespace events {
namespace state {

void
from_json(const json &obj, Topic &event)
{
        if (!obj.at("topic").is_null())
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
