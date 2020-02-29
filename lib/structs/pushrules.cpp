#include "mtx/pushrules.hpp"

#include <nlohmann/json.hpp>

namespace mtx {
namespace pushrules {

void
to_json(nlohmann::json &obj, const PushCondition &condition)
{
        obj["kind"] = condition.kind;
        if (!condition.key.empty())
                obj["key"] = condition.key;
        if (!condition.pattern.empty())
                obj["pattern"] = condition.pattern;
        if (!condition.is.empty())
                obj["is"] = condition.is;
}

void
from_json(const nlohmann::json &obj, PushCondition &condition)
{
        condition.kind    = obj["kind"];
        condition.key     = obj.value("key", "");
        condition.pattern = obj.value("pattern", "");
        condition.is      = obj.value("is", "");
}

namespace actions {
void
to_json(nlohmann::json &obj, const Action &action)
{
        if (std::holds_alternative<notify>(action))
                obj = "notify";
        else if (std::holds_alternative<dont_notify>(action))
                obj = "dont_notify";
        else if (auto n = std::get_if<set_tweak_sound>(&action)) {
                obj["set_tweak"] = "sound";
                obj["value"]     = n->value;
        } else if (auto h = std::get_if<set_tweak_highlight>(&action)) {
                obj["set_tweak"] = "highlight";
                if (h->value == false)
                        obj["value"] = false;
        }
}

void
from_json(const nlohmann::json &obj, Action &action)
{
        if (obj.is_string()) {
                if (obj == "notify")
                        action = notify{};
                else if (obj == "dont_notify")
                        action = dont_notify{};
        } else if (obj.contains("set_tweak")) {
                if (obj["set_tweak"] == "sound")
                        action = set_tweak_sound{obj.value("value", "default")};
                else if (obj["set_tweak"] == "highlight")
                        action = set_tweak_highlight{obj.value("value", true)};
        }
}

void
to_json(nlohmann::json &obj, const Actions &action)
{
        obj["actions"] = action.actions;
}

void
from_json(const nlohmann::json &obj, Actions &action)
{
        action.actions = obj["actions"].get<std::vector<Action>>();
}
}

void
to_json(nlohmann::json &obj, const PushRule &rule)
{
        if (rule.default_)
                obj["default"] = rule.default_;

        if (!rule.enabled)
                obj["enabled"] = rule.enabled;

        for (const auto &action : rule.actions)
                obj["actions"].push_back(action);

        if (!rule.rule_id.empty())
                obj["rule_id"] = rule.rule_id;

        if (!rule.pattern.empty())
                obj["pattern"] = rule.pattern;

        for (const auto condition : rule.conditions)
                obj["conditions"].push_back(condition);
}

void
from_json(const nlohmann::json &obj, PushRule &rule)
{
        rule.default_ = obj.value("default", false);
        rule.enabled  = obj.value("enabled", true);

        if (obj.contains("actions"))
                for (auto action : obj["actions"])
                        rule.actions.push_back(action);

        rule.pattern = obj.value("pattern", "");

        if (obj.contains("conditions"))
                for (auto condition : obj["conditions"])
                        rule.conditions.push_back(condition);
}

void
to_json(nlohmann::json &obj, const Ruleset &set)
{
        obj["override"]  = set.override_;
        obj["content"]   = set.content;
        obj["room"]      = set.room;
        obj["sender"]    = set.sender;
        obj["underride"] = set.underride;
}

void
from_json(const nlohmann::json &obj, Ruleset &set)
{
        if (obj.contains("override"))
                for (const auto e : obj["override"])
                        set.override_.push_back(e.get<PushRule>());
        if (obj.contains("content"))
                for (const auto e : obj["content"])
                        set.content.push_back(e.get<PushRule>());
        if (obj.contains("room"))
                for (const auto e : obj["room"])
                        set.room.push_back(e.get<PushRule>());
        if (obj.contains("sender"))
                for (const auto e : obj["sender"])
                        set.sender.push_back(e.get<PushRule>());
        if (obj.contains("underride"))
                for (const auto e : obj["underride"])
                        set.underride.push_back(e.get<PushRule>());
}
void
to_json(nlohmann::json &obj, const GlobalRuleset &set)
{
        obj["global"] = set.global;
}

void
from_json(const nlohmann::json &obj, GlobalRuleset &set)
{
        set.global = obj["global"];
}

void
to_json(nlohmann::json &obj, const Enabled &enabled)
{
        obj["enabled"] = enabled.enabled;
}

void
from_json(const nlohmann::json &obj, Enabled &enabled)
{
        enabled.enabled = obj["enabled"];
}
}
}
