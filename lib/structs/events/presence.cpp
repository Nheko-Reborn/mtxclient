#include "mtx/events/presence.hpp"

#include <nlohmann/json.hpp>

#include <string>
#include <string_view>

namespace mtx {
namespace presence {
std::string
to_string(PresenceState state)
{
    switch (state) {
    case PresenceState::offline:
        return "offline";
    case PresenceState::unavailable:
        return "unavailable";
    case PresenceState::online:
    default:
        return "online";
    }
}
PresenceState
from_string(std::string_view str)
{
    if (str == "offline")
        return PresenceState::offline;
    else if (str == "unavailable")
        return PresenceState::unavailable;
    else
        return PresenceState::online;
}
}

namespace events {
namespace presence {
void
from_json(const nlohmann::json &obj, Presence &presence)
{
    presence.avatar_url       = obj.value("avatar_url", "");
    presence.displayname      = obj.value("displayname", "");
    presence.last_active_ago  = obj.value("last_active_ago", std::uint64_t{0});
    presence.presence         = mtx::presence::from_string(obj.value("presence", "online"));
    presence.currently_active = obj.value("currently_active", false);
    try {
        if (obj.contains("status_msg"))
            presence.status_msg = obj.at("status_msg").get<std::string>();
    } catch (...) {
    }
}
void
to_json(nlohmann::json &obj, const Presence &presence)
{
    if (!presence.avatar_url.empty())
        obj["avatar_url"] = presence.avatar_url;
    if (!presence.displayname.empty())
        obj["displayname"] = presence.displayname;
    if (presence.last_active_ago)
        obj["last_active_ago"] = presence.last_active_ago;
    obj["presence"] = to_string(presence.presence);
    if (presence.currently_active)
        obj["currently_active"] = true;
    if (!presence.status_msg.empty())
        obj["status_msg"] = presence.status_msg;
}
}
}
}
