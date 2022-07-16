#include <nlohmann/json.hpp>
#include <string>

#include "mtx/events/join_rules.hpp"

using json = nlohmann::json;

namespace mtx {
namespace events {
namespace state {

std::string
joinRuleToString(const JoinRule &rule)
{
    switch (rule) {
    case JoinRule::Public:
        return "public";
    case JoinRule::Invite:
        return "invite";
    case JoinRule::Knock:
        return "knock";
    case JoinRule::Private:
        return "private";
    case JoinRule::Restricted:
        return "restricted";
    case JoinRule::KnockRestricted:
        return "knock_restricted";
    }

    return "";
}

JoinRule
stringToJoinRule(const std::string &rule)
{
    if (rule == "public")
        return JoinRule::Public;
    else if (rule == "invite")
        return JoinRule::Invite;
    else if (rule == "knock")
        return JoinRule::Knock;
    else if (rule == "restricted")
        return JoinRule::Restricted;
    else if (rule == "knock_restricted")
        return JoinRule::KnockRestricted;

    return JoinRule::Private;
}

void
from_json(const json &obj, JoinAllowance &allowance)
{
    if (obj.value("type", "") == "m.room_membership")
        allowance.type = JoinAllowanceType::RoomMembership;
    else
        allowance.type = JoinAllowanceType::Unknown;

    allowance.room_id = obj.value("room_id", "");
}

void
to_json(json &obj, const JoinAllowance &allowance)
{
    obj = nlohmann::json::object();
    if (allowance.type == JoinAllowanceType::RoomMembership) {
        obj["type"]    = "m.room_membership";
        obj["room_id"] = allowance.room_id;
    }
}

void
from_json(const json &obj, JoinRules &join_rules)
{
    join_rules.join_rule = stringToJoinRule(obj.value("join_rule", ""));

    if (obj.contains("allow"))
        join_rules.allow = obj.at("allow").get<decltype(join_rules.allow)>();
}

void
to_json(json &obj, const JoinRules &join_rules)
{
    obj["join_rule"] = joinRuleToString(join_rules.join_rule);

    if (!join_rules.allow.empty())
        obj["allow"] = join_rules.allow;
}

} // namespace state
} // namespace events
} // namespace mtx
