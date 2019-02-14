#pragma once

#include <nlohmann/json.hpp>
#include <string>

using json = nlohmann::json;

namespace mtx {
namespace events {
namespace state {

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

        /* ThirdPartyInvite third_party_invite; */
};

void
from_json(const json &obj, Member &member);

void
to_json(json &obj, const Member &member);

} // namespace state
} // namespace events
} // namespace mtx
