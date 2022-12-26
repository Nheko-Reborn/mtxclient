#include <nlohmann/json.hpp>
#include <string>

#include "mtx/events/member.hpp"

using json = nlohmann::json;

namespace mtx {
namespace events {
namespace state {

std::string
membershipToString(const Membership &membership)
{
    switch (membership) {
    case Membership::Join:
        return "join";
    case Membership::Invite:
        return "invite";
    case Membership::Ban:
        return "ban";
    case Membership::Leave:
        return "leave";
    case Membership::Knock:
        return "knock";
    }

    return "";
}

Membership
stringToMembership(const std::string &membership)
{
    if (membership == "join")
        return Membership::Join;
    else if (membership == "invite")
        return Membership::Invite;
    else if (membership == "ban")
        return Membership::Ban;
    else if (membership == "leave")
        return Membership::Leave;

    return Membership::Knock;
}

void
from_json(const json &obj, Member &member)
{
    member.membership = stringToMembership(obj.at("membership").get<std::string>());

    if (obj.count("displayname") != 0 && !obj.at("displayname").is_null())
        member.display_name = obj.at("displayname").get<std::string>();

    if (obj.count("avatar_url") != 0 && !obj.at("avatar_url").is_null())
        member.avatar_url = obj.at("avatar_url").get<std::string>();

    if (obj.find("is_direct") != obj.end())
        member.is_direct = obj.at("is_direct").get<bool>();

    if (auto r = obj.find("reason"); r != obj.end() && r->is_string())
        member.reason = obj.at("reason").get<std::string>();

    if (obj.contains("join_authorised_via_users_server"))
        member.join_authorised_via_users_server =
          obj.at("join_authorised_via_users_server").get<std::string>();
}

void
to_json(json &obj, const Member &member)
{
    obj["membership"]  = membershipToString(member.membership);
    obj["avatar_url"]  = member.avatar_url;
    obj["displayname"] = member.display_name;
    obj["is_direct"]   = member.is_direct;

    if (!member.reason.empty())
        obj["reason"] = member.reason;

    if (!member.join_authorised_via_users_server.empty())
        obj["join_authorised_via_users_server"] = member.join_authorised_via_users_server;
}

} // namespace state
} // namespace events
} // namespace mtx
