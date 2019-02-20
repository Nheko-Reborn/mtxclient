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
        else if (rule == "Knock")
                return JoinRule::Knock;

        return JoinRule::Private;
}

void
from_json(const json &obj, JoinRules &join_rules)
{
        join_rules.join_rule = stringToJoinRule(obj.at("join_rule").get<std::string>());
}

void
to_json(json &obj, const JoinRules &join_rules)
{
        obj["join_rule"] = joinRuleToString(join_rules.join_rule);
}

} // namespace state
} // namespace events
} // namespace mtx
