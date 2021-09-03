#include <nlohmann/json.hpp>
#include <string>

#include "mtx/events/name.hpp"

using json = nlohmann::json;

namespace mtx {
namespace events {
namespace state {

void
from_json(const json &obj, Name &event)
{
    if (obj.find("name") != obj.end() && !obj.at("name").is_null())
        event.name = obj.at("name").get<std::string>();
}

void
to_json(json &obj, const Name &event)
{
    obj["name"] = event.name;
}

} // namespace state
} // namespace events
} // namespace mtx
