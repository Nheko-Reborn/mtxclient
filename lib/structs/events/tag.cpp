#include "mtx/events/tag.hpp"

#include <nlohmann/json.hpp>

using json = nlohmann::json;

namespace mtx {
namespace events {
namespace account_data {

void
from_json(const json &obj, Tag &content)
{
    if (obj.contains("order"))
        content.order = obj.at("order").get<double>();
}

void
to_json(json &obj, const Tag &content)
{
    obj = nlohmann::json::object();
    if (content.order)
        obj["order"] = content.order.value();
}

void
from_json(const json &obj, Tags &content)
{
    content.tags = obj.at("tags").get<std::map<std::string, Tag>>();
}

void
to_json(json &obj, const Tags &content)
{
    obj["tags"] = content.tags;
}

} // namespace state
} // namespace events
} // namespace mtx
