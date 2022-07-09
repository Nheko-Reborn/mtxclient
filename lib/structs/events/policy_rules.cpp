#include "mtx/events/policy_rules.hpp"

#include <nlohmann/json.hpp>

namespace mtx::events::state::policy_rule {
void
from_json(const nlohmann::json &obj, Rule &rule)
{
    rule.entity         = obj.at("entity").get<std::string>();
    rule.recommendation = obj.at("recommendation").get<std::string>();
    rule.reason         = obj.at("reason").get<std::string>();
}

void
to_json(nlohmann::json &obj, const Rule &rule)
{
    obj = nlohmann::json{
      {"entity", rule.entity},
      {"recommendation", rule.recommendation},
      {"reason", rule.reason},
    };
}

void
from_json(const nlohmann::json &obj, UserRule &rule)
{
    from_json(obj, static_cast<Rule &>(rule));
}
void
to_json(nlohmann::json &obj, const UserRule &rule)
{
    to_json(obj, static_cast<const Rule &>(rule));
}
void
from_json(const nlohmann::json &obj, RoomRule &rule)
{
    from_json(obj, static_cast<Rule &>(rule));
}
void
to_json(nlohmann::json &obj, const RoomRule &rule)
{
    to_json(obj, static_cast<const Rule &>(rule));
}
void
from_json(const nlohmann::json &obj, ServerRule &rule)
{
    from_json(obj, static_cast<Rule &>(rule));
}
void
to_json(nlohmann::json &obj, const ServerRule &rule)
{
    to_json(obj, static_cast<const Rule &>(rule));
}
}
