#include "mtx/events/spaces.hpp"

#include <string>

#include <nlohmann/json.hpp>

namespace mtx {
namespace events {
namespace state {
namespace space {

void
from_json(const nlohmann::json &obj, Parent &parent)
{
    if (obj.contains("canonical") && obj.at("canonical").is_boolean())
        parent.canonical = obj.at("canonical").get<bool>();
    if (obj.contains("via") && obj.at("via").is_array() && !obj.at("via").empty())
        parent.via = obj.at("via").get<std::vector<std::string>>();
}

void
to_json(nlohmann::json &obj, const Parent &parent)
{
    obj = nlohmann::json::object();

    // event without via is invalid.
    if (!parent.via.has_value() || parent.via.value().empty())
        return;

    obj["via"] = parent.via.value();

    if (parent.canonical)
        obj["canonical"] = true;
}

static bool
is_valid_order_str(std::string_view order)
{
    if (order.size() > 50)
        return false;

    for (auto c : order)
        if (c < '\x20' || c > '\x7E')
            return false;

    return true;
}

void
from_json(const nlohmann::json &obj, Child &child)
{
    if (obj.contains("via") && obj.at("via").is_array() && !obj.at("via").empty())
        child.via = obj.at("via").get<std::vector<std::string>>();

    if (obj.contains("order") && obj.at("order").is_string() &&
        is_valid_order_str(obj.at("order").get<std::string>()))
        child.order = obj.at("order").get<std::string>();

    child.suggested = obj.value("suggested", false);
}

void
to_json(nlohmann::json &obj, const Child &child)
{
    obj = nlohmann::json::object();

    // event without via is invalid.
    if (!child.via.has_value() || child.via.value().empty())
        return;

    obj["via"] = child.via.value();

    if (child.order && is_valid_order_str(child.order.value()))
        obj["order"] = child.order.value();

    if (child.suggested)
        obj["suggested"] = true;
}
}
} // namespace state
} // namespace events
} // namespace mtx
