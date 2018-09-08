#include <string>

#include "mtx/events/tag.hpp"

using json = nlohmann::json;

namespace mtx {
namespace events {
namespace account_data {

void
from_json(const json &obj, Tag &content)
{
        content.tags = obj.at("tags").get<std::map<std::string, json>>();
}

void
to_json(json &obj, const Tag &content)
{
        obj["tags"] = content.tags;
}

} // namespace state
} // namespace events
} // namespace mtx
