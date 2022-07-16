#include <nlohmann/json.hpp>
#include <string>

#include "mtx/events/history_visibility.hpp"

using json = nlohmann::json;

namespace mtx {
namespace events {
namespace state {

std::string
visibilityToString(const Visibility &rule)
{
    switch (rule) {
    case Visibility::WorldReadable:
        return "world_readable";
    case Visibility::Invited:
        return "invited";
    case Visibility::Shared:
        return "shared";
    case Visibility::Joined:
        return "joined";
    }

    return "";
}

Visibility
stringToVisibility(const std::string &rule)
{
    if (rule == "world_readable")
        return Visibility::WorldReadable;
    else if (rule == "invited")
        return Visibility::Invited;
    else if (rule == "shared")
        return Visibility::Shared;

    return Visibility::Joined;
}

void
from_json(const json &obj, HistoryVisibility &event)
{
    event.history_visibility = stringToVisibility(obj.value("history_visibility", ""));
}

void
to_json(json &obj, const HistoryVisibility &event)
{
    obj["history_visibility"] = visibilityToString(event.history_visibility);
}

} // namespace state
} // namespace events
} // namespace mtx
