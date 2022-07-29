#pragma once

/// @file
/// @brief The state event describing the membership status of a specific member.

#if __has_include(<nlohmann/json_fwd.hpp>)
#include <nlohmann/json_fwd.hpp>
#else
#include <nlohmann/json.hpp>
#endif

#include <string>

namespace mtx {
namespace events {
namespace state {
//! The different Membership states.
enum class Membership
{
    //! The user has joined.
    Join,
    //! The user has been invited.
    Invite,
    //! The user is banned.
    Ban,
    //! The user has left.
    Leave,
    //! The user has requested to join.
    Knock,
};

std::string
membershipToString(const Membership &membership);

Membership
stringToMembership(const std::string &membership);

//! Content of the `m.room.member` state event.
struct Member
{
    //! The membership state of the user.
    Membership membership;
    //! The avatar URL for this user, if any.
    std::string avatar_url;
    //! The display name for this user, if any.
    std::string display_name;
    //! Flag indicating if the room containing this event was created
    //! with the intention of being a direct chat.
    bool is_direct = false;

    //! reason for the membership change, empty in most cases
    std::string reason;

    //! In a restricted room, on what user was used to authorize the join.
    std::string join_authorised_via_users_server;

    /* ThirdPartyInvite third_party_invite; */

    friend void from_json(const nlohmann::json &obj, Member &member);
    friend void to_json(nlohmann::json &obj, const Member &member);
};

} // namespace state
} // namespace events
} // namespace mtx
