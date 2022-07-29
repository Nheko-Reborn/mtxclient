#include "mtx/responses/capabilities.hpp"

#include <nlohmann/json.hpp>

namespace mtx::responses::capabilities {
void
from_json(const nlohmann::json &obj, RoomVersionStability &stab)
{
    if (obj == "stable")
        stab = RoomVersionStability::Stable;
    else
        stab = RoomVersionStability::Unstable;
}

void
from_json(const nlohmann::json &obj, RoomVersionsCapability &cap)
{
    cap.available = obj.value("available",
                              std::map<std::string, RoomVersionStability>{
                                {"1", RoomVersionStability::Stable},
                              });
    cap.default_  = obj.value("default", "1");
}

void
from_json(const nlohmann::json &obj, Enabled &cap)
{
    cap.enabled = obj.value("enabled", true);
}

void
from_json(const nlohmann::json &obj, Capabilities &caps)
{
    if (obj.contains("capabilities") && obj.at("capabilities").is_object()) {
        const auto &j = obj.at("capabilities");

        caps.room_versions   = j.value("m.room_versions", RoomVersionsCapability{});
        caps.change_password = j.value("m.change_password", Enabled{});
        caps.set_displayname = j.value("m.set_displayname", Enabled{});
        caps.set_avatar_url  = j.value("m.set_avatar_url", Enabled{});
        caps.change_3pid     = j.value("m.3pid_changes", Enabled{});
    }
}
}
