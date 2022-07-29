#pragma once

/// @file
/// @brief Responses from the capabilities API.

#if __has_include(<nlohmann/json_fwd.hpp>)
#include <nlohmann/json_fwd.hpp>
#else
#include <nlohmann/json.hpp>
#endif

namespace mtx {
namespace responses {
namespace capabilities {
//! Any version not explicitly labelled as stable in the available versions is to be treated as
//! unstable. For example, a version listed as future-stable should be treated as unstable.
enum class RoomVersionStability
{
    Unstable, //!< Any room version not listed as stable.
    Stable,   //!< A stable room versions
};
void
from_json(const nlohmann::json &obj, RoomVersionStability &stab);

//! The room versions the server supports.
struct RoomVersionsCapability
{
    //! Required: The default room version the server is using for new rooms.
    std::string default_ = "1";
    //! Required: A detailed description of the room versions the server supports.
    std::map<std::string, RoomVersionStability> available = {
      {"1", RoomVersionStability::Stable},
    };

    friend void from_json(const nlohmann::json &obj, RoomVersionsCapability &cap);
};

//! any capability that can be enabled or disabled.
struct Enabled
{
    //! If this capability is enabled.
    bool enabled = true;

    friend void from_json(const nlohmann::json &obj, Enabled &cap);
};

//! Response from the `GET  /_matrix/client/v3/capabilities` endpoint.
//
//! Gets information about the server’s supported feature set and other relevant capabilities.
struct Capabilities
{
    //! The room versions the server supports.
    RoomVersionsCapability room_versions;
    //! Capability to indicate if the user can change their password.
    Enabled change_password;
    //! Capability to indicate if the user can change their displayname.
    Enabled set_displayname;
    //! Capability to indicate if the user can change their avatar.
    Enabled set_avatar_url;
    //! This capability has a single flag, enabled, to denote whether the user is able to add,
    //! remove, or change 3PID associations on their account.
    Enabled change_3pid;

    friend void from_json(const nlohmann::json &obj, Capabilities &caps);
};
}
}
}
