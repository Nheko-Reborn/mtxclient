#pragma once

/// @file
/// @brief A nheko specific event in account data used to block or allow list invites.

#include <map>
#include <string>

#if __has_include(<nlohmann/json_fwd.hpp>)
#include <nlohmann/json_fwd.hpp>
#else
#include <nlohmann/json.hpp>
#endif

namespace mtx {
namespace events {
namespace account_data {
namespace nheko_extensions {
//! Custom event to allow or block invites.
struct InvitePermissions
{
    //! Default permissions
    std::string default_;

    //! Server allowed to invite
    std::map<std::string, std::string, std::less<>> server_allow;
    //! Server denied from inviting
    std::map<std::string, std::string, std::less<>> server_deny;

    //! Rooms the user accepts invites to
    std::map<std::string, std::string, std::less<>> room_allow;
    //! Rooms the user blocks invites to
    std::map<std::string, std::string, std::less<>> room_deny;

    //! Users the user is accepting invites from
    std::map<std::string, std::string, std::less<>> user_allow;
    //! Users the user is blocking invites from
    std::map<std::string, std::string, std::less<>> user_deny;

    [[nodiscard]] bool invite_allowed(std::string_view room_id, std::string_view inviter) const;

    friend void from_json(const nlohmann::json &obj, InvitePermissions &content);
    friend void to_json(nlohmann::json &obj, const InvitePermissions &content);
};
}
} // namespace account_data
} // namespace events
} // namespace mtx
