#include "mtx/pushrules.hpp"

#include <charconv>

#include <nlohmann/json.hpp>
#include <re2/re2.h>

#include "mtx/events/collections.hpp"
#include "mtx/log.hpp"

namespace {
struct RelatedEvents
{
    std::vector<std::unordered_map<std::string, std::string>>
      fallbacks;                                                      //!< fallback related events
    std::vector<std::unordered_map<std::string, std::string>> events; //!< related events
};
}

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
    if (condition.rel_type != mtx::common::RelationType::Unsupported)
        obj["rel_type"] = condition.rel_type;
}

void
from_json(const nlohmann::json &obj, PushCondition &condition)
{
    condition.kind             = obj["kind"].get<std::string>();
    condition.key              = obj.value("key", "");
    condition.pattern          = obj.value("pattern", "");
    condition.is               = obj.value("is", "");
    condition.rel_type         = obj.value("rel_type", mtx::common::RelationType::Unsupported);
    condition.include_fallback = obj.value("include_fallback", false);
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

    for (const auto &condition : rule.conditions)
        obj["conditions"].push_back(condition);
}

void
from_json(const nlohmann::json &obj, PushRule &rule)
{
    rule.rule_id  = obj.value("rule_id", "");
    rule.default_ = obj.value("default", false);
    rule.enabled  = obj.value("enabled", true);

    if (obj.contains("actions"))
        for (const auto &action : obj["actions"])
            rule.actions.push_back(action.get<actions::Action>());

    rule.pattern = obj.value("pattern", "");

    if (obj.contains("conditions"))
        for (const auto &condition : obj["conditions"])
            rule.conditions.push_back(condition.get<PushCondition>());
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
        for (const auto &e : obj["override"])
            set.override_.push_back(e.get<PushRule>());
    if (obj.contains("content"))
        for (const auto &e : obj["content"])
            set.content.push_back(e.get<PushRule>());
    if (obj.contains("room"))
        for (const auto &e : obj["room"])
            set.room.push_back(e.get<PushRule>());
    if (obj.contains("sender"))
        for (const auto &e : obj["sender"])
            set.sender.push_back(e.get<PushRule>());
    if (obj.contains("underride"))
        for (const auto &e : obj["underride"])
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
    set.global = obj["global"].get<Ruleset>();
}

void
to_json(nlohmann::json &obj, const Enabled &enabled)
{
    obj["enabled"] = enabled.enabled;
}

void
from_json(const nlohmann::json &obj, Enabled &enabled)
{
    enabled.enabled = obj.value("enabled", true);
}

struct PushRuleEvaluator::OptimizedRules
{
    //! The individual rule to apply
    struct OptimizedRule
    {
        //! a pattern condition to match
        struct PatternCondition
        {
            std::unique_ptr<re2::RE2> pattern; //!< the pattern
            std::string field;                 //!< the field to match with pattern

            bool matches(const std::unordered_map<std::string, std::string> &ev) const
            {
                if (auto it = ev.find(field); it != ev.end()) {
                    if (pattern) {
                        if (field == "content.body") {
                            if (!re2::RE2::PartialMatch(it->second, *pattern))
                                return false;
                        } else {
                            if (!re2::RE2::FullMatch(it->second, *pattern))
                                return false;
                        }
                    }
                } else {
                    return false;
                }

                return true;
            }
        };
        // TODO(Nico): Sort by field for faster matching?
        std::vector<PatternCondition> patterns; //!< conditions that match on a field

        //! a pattern condition to match on a related event
        struct RelatedEventCondition
        {
            PatternCondition ev_match;
            mtx::common::RelationType rel_type = mtx::common::RelationType::Unsupported;
            bool include_fallbacks             = false;
        };
        std::vector<RelatedEventCondition>
          related_event_patterns; //!< conditions that match on fields of the related event.

        //! a member count condition
        struct MemberCountCondition
        {
            //! the count to compare against
            std::size_t count = 0;
            //! the comparison operation
            enum Comp
            {
                Eq, //< ==
                Lt, //< <
                Le, //< <=
                Ge, //< >=
                Gt, //< >
            };

            Comp op = Comp::Eq;
        };
        std::vector<MemberCountCondition> membercounts; //< conditions that match on member count

        std::vector<std::string> notification_levels;

        //! evaluate contains_display_name condition
        bool check_displayname = false;

        std::vector<actions::Action> actions; //< the actions to apply on match

        [[nodiscard]] bool matches(
          const std::unordered_map<std::string, std::string> &ev,
          const PushRuleEvaluator::RoomContext &ctx,
          const std::map<mtx::common::RelationType, RelatedEvents> &relatedEventsFlat) const
        {
            for (const auto &cond : membercounts) {
                if (![&cond, &ctx] {
                        switch (cond.op) {
                        case MemberCountCondition::Eq:
                            return ctx.member_count == cond.count;
                        case MemberCountCondition::Le:
                            return ctx.member_count <= cond.count;
                        case MemberCountCondition::Ge:
                            return ctx.member_count >= cond.count;
                        case MemberCountCondition::Lt:
                            return ctx.member_count < cond.count;
                        case MemberCountCondition::Gt:
                            return ctx.member_count > cond.count;
                        default:
                            return false;
                        }
                    }())
                    return false;
            }

            if (!notification_levels.empty()) {
                auto sender_ = ev.find("sender");
                if (sender_ == ev.end())
                    return false;

                auto sender_level = ctx.power_levels.user_level(sender_->second);

                for (const auto &n : notification_levels) {
                    if (sender_level < ctx.power_levels.notification_level(n))
                        return false;
                }
            }

            for (const auto &cond : patterns) {
                if (!cond.matches(ev))
                    return false;
            }

            for (const auto &cond : related_event_patterns) {
                bool matched = false;
                for (const auto &[rel_type, rel_ev] : relatedEventsFlat) {
                    if (cond.rel_type == rel_type) {
                        for (const auto &e : rel_ev.events) {
                            if (cond.ev_match.field.empty() || !cond.ev_match.pattern ||
                                cond.ev_match.matches(e)) {
                                matched = true;
                                break;
                            }
                        }
                        if (cond.include_fallbacks) {
                            for (const auto &e : rel_ev.fallbacks) {
                                if (cond.ev_match.field.empty() || !cond.ev_match.pattern ||
                                    cond.ev_match.matches(e)) {
                                    matched = true;
                                    break;
                                }
                            }
                        }
                    }
                }
                if (!matched)
                    return false;
            }

            if (check_displayname) {
                if (ctx.user_display_name.empty())
                    return false;

                if (auto it = ev.find("content.body"); it != ev.end()) {
                    re2::RE2::Options opts;
                    opts.set_case_sensitive(false);

                    if (!re2::RE2::PartialMatch(
                          it->second,
                          re2::RE2("(\\W|^)" + re2::RE2::QuoteMeta(ctx.user_display_name) +
                                     "(\\W|$)",
                                   opts)))
                        return false;
                } else {
                    return false;
                }
            }

            return true;
        }
    };

    std::vector<OptimizedRule> override_;
    std::unordered_map<std::string, OptimizedRule> room;
    std::unordered_map<std::string, OptimizedRule> sender;
    std::vector<OptimizedRule> content;
    std::vector<OptimizedRule> underride;
};

static std::unique_ptr<re2::RE2>
construct_re_from_pattern(std::string pat, const std::string &field)
{
    pat = re2::RE2::QuoteMeta(pat);

    // Quote also espaces the globs, so we need to match them including the backslash
    static re2::RE2 matchGlobStar("\\*");
    re2::RE2::GlobalReplace(&pat, matchGlobStar, ".*");

    static re2::RE2 matchGlobQuest("\\?");
    re2::RE2::GlobalReplace(&pat, matchGlobQuest, ".");

    re2::RE2::Options opts;
    opts.set_case_sensitive(false);

    if (field == "content.body")
        return std::make_unique<re2::RE2>("(\\W|^)" + pat + "(\\W|$)", opts);
    else
        return std::make_unique<re2::RE2>(pat, opts);
}

PushRuleEvaluator::~PushRuleEvaluator() = default;
PushRuleEvaluator::PushRuleEvaluator(const Ruleset &rules_)
  : rules(std::make_unique<OptimizedRules>())
{
    auto add_conditions_to_rule = [](OptimizedRules::OptimizedRule &rule,
                                     const std::vector<PushCondition> &conditions) {
        for (const auto &cond : conditions) {
            if (cond.kind == "event_match") {
                OptimizedRules::OptimizedRule::PatternCondition c;
                c.field   = cond.key;
                c.pattern = construct_re_from_pattern(cond.pattern, cond.key);
                if (c.pattern)
                    rule.patterns.push_back(std::move(c));
            } else if (cond.kind == "im.nheko.msc3664.related_event_match") {
                OptimizedRules::OptimizedRule::RelatedEventCondition c;

                if (cond.rel_type != mtx::common::RelationType::Unsupported) {
                    c.rel_type          = cond.rel_type;
                    c.include_fallbacks = cond.include_fallback;

                    if (!cond.key.empty() && !cond.pattern.empty()) {
                        c.ev_match.field   = cond.key;
                        c.ev_match.pattern = construct_re_from_pattern(cond.pattern, cond.key);
                    }
                    rule.related_event_patterns.push_back(std::move(c));
                } else {
                    mtx::utils::log::log()->info(
                      "Skipping rel_event_match rule with unknown rel_type.");
                    return false;
                }
            } else if (cond.kind == "contains_display_name") {
                rule.check_displayname = true;
            } else if (cond.kind == "room_member_count") {
                OptimizedRules::OptimizedRule::MemberCountCondition c;
                std::string_view is = cond.is;
                if (is.starts_with("==")) {
                    c.op = c.Comp::Eq;
                    is   = is.substr(2);
                } else if (is.starts_with(">=")) {
                    c.op = c.Comp::Ge;
                    is   = is.substr(2);
                } else if (is.starts_with("<=")) {
                    c.op = c.Comp::Le;
                    is   = is.substr(2);
                } else if (is.starts_with('<')) {
                    c.op = c.Comp::Lt;
                    is   = is.substr(1);
                } else if (is.starts_with('>')) {
                    c.op = c.Comp::Gt;
                    is   = is.substr(1);
                }

                std::from_chars(is.data(), is.data() + is.size(), c.count);
                rule.membercounts.push_back(c);
            } else if (cond.kind == "sender_notification_permission") {
                rule.notification_levels.push_back(cond.key);
            } else {
                mtx::utils::log::log()->info("Skipping rule with unknown condition type: {}",
                                             cond.kind);
                return false;
            }
        }

        return true;
    };

    for (const auto &rule_ : rules_.override_) {
        if (!rule_.enabled)
            continue;

        OptimizedRules::OptimizedRule rule;
        rule.actions = rule_.actions;

        if (!add_conditions_to_rule(rule, rule_.conditions))
            continue;

        rules->override_.push_back(std::move(rule));
    }

    for (const auto &rule_ : rules_.underride) {
        if (!rule_.enabled)
            continue;

        OptimizedRules::OptimizedRule rule;
        rule.actions = rule_.actions;

        if (!add_conditions_to_rule(rule, rule_.conditions))
            continue;

        rules->underride.push_back(std::move(rule));
    }

    for (const auto &rule_ : rules_.room) {
        if (!rule_.enabled)
            continue;

        if (!rule_.rule_id.starts_with("!"))
            continue;

        OptimizedRules::OptimizedRule rule;
        rule.actions               = rule_.actions;
        rules->room[rule_.rule_id] = std::move(rule);
    }

    for (const auto &rule_ : rules_.sender) {
        if (!rule_.enabled)
            continue;

        if (!rule_.rule_id.starts_with("@"))
            continue;

        OptimizedRules::OptimizedRule rule;
        rule.actions                 = rule_.actions;
        rules->sender[rule_.rule_id] = std::move(rule);
    }

    for (const auto &rule_ : rules_.content) {
        if (!rule_.enabled)
            continue;

        // Work around construct sending invalid content rules.
        // Also this seems like just a sane thing to do, an empty pattern will always match and that
        // is usually not what you want.
        if (rule_.pattern.empty())
            continue;

        OptimizedRules::OptimizedRule rule;
        rule.actions = rule_.actions;

        std::vector<PushCondition> conditions{
          PushCondition{.kind = "event_match", .key = "content.body", .pattern = rule_.pattern},
        };

        if (!add_conditions_to_rule(rule, conditions))
            continue;

        rules->content.push_back(std::move(rule));
    }
}

static void
flatten_impl(const nlohmann::json &value,
             std::unordered_map<std::string, std::string> &result,
             const std::string &current_path,
             int current_depth)
{
    if (current_depth > 100)
        return;

    switch (value.type()) {
    case nlohmann::json::value_t::object: {
        // iterate object and use keys as reference string
        std::string prefix;
        if (!current_path.empty())
            prefix = current_path + ".";
        for (const auto &element : value.items()) {
            flatten_impl(element.value(), result, prefix + element.key(), current_depth + 1);
        }
        break;
    }

    case nlohmann::json::value_t::string: {
        // add primitive value with its reference string
        result[current_path] = value.get<std::string>();
        break;
    }

        // currently we only match strings
    case nlohmann::json::value_t::array:
    case nlohmann::json::value_t::null:
    case nlohmann::json::value_t::boolean:
    case nlohmann::json::value_t::number_integer:
    case nlohmann::json::value_t::number_unsigned:
    case nlohmann::json::value_t::number_float:
    case nlohmann::json::value_t::binary:
    case nlohmann::json::value_t::discarded:
    default:
        break;
    }
}

static std::unordered_map<std::string, std::string>
flatten_event(const nlohmann::json &j)
{
    std::unordered_map<std::string, std::string> flat;
    flatten_impl(j, flat, "", 0);
    return flat;
}

std::vector<actions::Action>
PushRuleEvaluator::evaluate(
  const mtx::events::collections::TimelineEvent &event,
  const RoomContext &ctx,
  const std::vector<std::pair<mtx::common::Relation, mtx::events::collections::TimelineEvent>>
    &relatedEvents) const
{
    auto event_json = nlohmann::json(event);
    auto flat_event = flatten_event(event_json);

    std::map<mtx::common::RelationType, RelatedEvents> relatedEventsFlat;
    for (const auto &[rel, ev] : relatedEvents) {
        if (rel.rel_type != mtx::common::RelationType::Unsupported) {
            if (rel.is_fallback)
                relatedEventsFlat[rel.rel_type].fallbacks.push_back(
                  flatten_event(nlohmann::json(ev)));
            else
                relatedEventsFlat[rel.rel_type].events.push_back(flatten_event(nlohmann::json(ev)));
        }
    }

    for (const auto &rule : rules->override_) {
        if (rule.matches(flat_event, ctx, relatedEventsFlat))
            return rule.actions;
    }

    for (const auto &rule : rules->content) {
        if (rule.matches(flat_event, ctx, relatedEventsFlat))
            return rule.actions;
    }

    // room rule always matches if present
    if (auto room_rule = rules->room.find(event_json.value("room_id", ""));
        room_rule != rules->room.end()) {
        return room_rule->second.actions;
    }

    // sender rule always matches if present
    if (auto sender_rule = rules->sender.find(event_json.value("sender", ""));
        sender_rule != rules->sender.end()) {
        return sender_rule->second.actions;
    }

    for (const auto &rule : rules->underride) {
        if (rule.matches(flat_event, ctx, relatedEventsFlat))
            return rule.actions;
    }
    return {};
}

}
}
