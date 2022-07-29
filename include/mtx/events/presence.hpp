#pragma once

/// @file
/// @brief An ephemeral event describing the presence of a user.

#if __has_include(<nlohmann/json_fwd.hpp>)
#include <nlohmann/json_fwd.hpp>
#else
#include <nlohmann/json.hpp>
#endif

#include <optional>
#include <string>
#include <string_view>

namespace mtx {
//! Presence specific types.
namespace presence {
//! The current presence state.
enum PresenceState
{
    online,      //!< The user is online.
    offline,     //!< The user is offline.
    unavailable, //!< The user is online, but currently not available.
};

std::string
to_string(PresenceState state);
PresenceState
from_string(std::string_view str);
}

namespace events {
namespace presence {
//! The `m.presence` ephemeral event.
struct Presence
{
    std::string avatar_url;  //! The current avatar URL for this user, if any.
    std::string displayname; //! The current display name for this user, if any.
    uint64_t
      last_active_ago; //! The last time since this used performed some action, in milliseconds.
    mtx::presence::PresenceState presence; //! Required. The presence state for this user. One
                                           //! of: ["online", "offline", "unavailable"]
    bool currently_active;                 //! Whether the user is currently active
    std::string status_msg;                //! An optional description to accompany the presence.

    friend void from_json(const nlohmann::json &obj, Presence &presence);
    friend void to_json(nlohmann::json &obj, const Presence &presence);
};
}
}
}
