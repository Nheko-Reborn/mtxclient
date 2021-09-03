#include <nlohmann/json.hpp>
#include <string>

#include "mtx/events/power_levels.hpp"

using json = nlohmann::json;

namespace mtx {
namespace events {
namespace state {

void
from_json(const json &obj, PowerLevels &power_levels)
{
    // SPEC_BUG: Not always present.
    if (obj.count("ban") != 0)
        power_levels.ban = obj.at("ban").get<power_level_t>();

    if (obj.count("invite") != 0)
        power_levels.invite = obj.at("invite").get<power_level_t>();

    if (obj.count("kick") != 0)
        power_levels.kick = obj.at("kick").get<power_level_t>();

    if (obj.count("redact") != 0)
        power_levels.redact = obj.at("redact").get<power_level_t>();

    if (obj.count("events") != 0)
        power_levels.events =
          obj.at("events").get<std::map<std::string, power_level_t, std::less<>>>();
    if (obj.count("users") != 0)
        power_levels.users =
          obj.at("users").get<std::map<std::string, power_level_t, std::less<>>>();

    if (obj.count("events_default") != 0)
        power_levels.events_default = obj.at("events_default").get<power_level_t>();
    if (obj.count("users_default") != 0)
        power_levels.users_default = obj.at("users_default").get<power_level_t>();
    if (obj.count("state_default") != 0)
        power_levels.state_default = obj.at("state_default").get<power_level_t>();
    if (obj.contains("notifications"))
        power_levels.notifications =
          obj.at("notifications").get<std::map<std::string, power_level_t, std::less<>>>();
}

void
to_json(json &obj, const PowerLevels &power_levels)
{
    obj["ban"]    = power_levels.ban;
    obj["kick"]   = power_levels.kick;
    obj["invite"] = power_levels.invite;
    obj["redact"] = power_levels.redact;

    if (power_levels.events.size() != 0)
        obj["events"] = power_levels.events;
    if (power_levels.users.size() != 0)
        obj["users"] = power_levels.users;

    obj["events_default"] = power_levels.events_default;
    obj["users_default"]  = power_levels.users_default;
    obj["state_default"]  = power_levels.state_default;

    if (!power_levels.notifications.empty())
        obj["notifications"] = power_levels.notifications;
}

} // namespace state
} // namespace events
} // namespace mtx
