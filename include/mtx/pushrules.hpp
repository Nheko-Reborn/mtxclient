#pragma once

/// @file
/// @brief Pushrules and notification settings.

#if __has_include(<nlohmann/json_fwd.hpp>)
#include <nlohmann/json_fwd.hpp>
#else
#include <nlohmann/json.hpp>
#endif

#include <string>
#include <variant>
#include <vector>

namespace mtx {
//! Namespace for the pushrules specific endpoints.
namespace pushrules {
//! A condition to match pushrules on.
struct PushCondition
{
    //! Required. The kind of condition to apply. See conditions for more information on the
    //! allowed kinds and how they work.
    std::string kind;
    //! Required for event_match conditions. The dot- separated field of the event to match.
    //!
    //! Required for sender_notification_permission conditions. The field in the power level
    //! event the user needs a minimum power level for. Fields must be specified under the
    //! notifications property in the power level event's content.
    std::string key;
    //! Required for event_match conditions. The glob- style pattern to match against. Patterns
    //! with no special glob characters should be treated as having asterisks prepended and
    //! appended when testing the condition.
    std::string pattern;
    //! Required for room_member_count conditions. A decimal integer optionally prefixed by one
    //! of, ==, <, >, >= or <=. A prefix of < matches rooms where the member count is strictly
    //! less than the given number and so forth. If no prefix is present, this parameter
    //! defaults to ==.
    std::string is;

    friend void to_json(nlohmann::json &obj, const PushCondition &condition);
    friend void from_json(const nlohmann::json &obj, PushCondition &condition);
};
//! Namespace for the different push actions.
namespace actions {
//! Notify the user.
struct notify
{};
//! Don't notify the user.
struct dont_notify
{};
/// @brief This enables notifications for matching events but activates homeserver specific
/// behaviour to intelligently coalesce multiple events into a single notification.
///
/// Not all homeservers may support this. Those that do not support it should treat it as the notify
/// action.
struct coalesce
{};
//! Play a sound.
struct set_tweak_sound
{
    //! The sound to play.
    std::string value = "default";
};
//! Highlight the message.
struct set_tweak_highlight
{
    bool value = true;
};

//! A collection for the different actions.
using Action = std::variant<actions::notify,
                            actions::dont_notify,
                            actions::coalesce,
                            actions::set_tweak_sound,
                            actions::set_tweak_highlight>;

void
to_json(nlohmann::json &obj, const Action &action);

void
from_json(const nlohmann::json &obj, Action &action);

//! A list of actions.
struct Actions
{
    std::vector<Action> actions;

    friend void to_json(nlohmann::json &obj, const Actions &action);
    friend void from_json(const nlohmann::json &obj, Actions &action);
};
}

//! A pushrule defining the notification behaviour for a message.
struct PushRule
{
    //! Required. Whether this is a default rule, or has been set explicitly.
    bool default_ = false;
    //! Required. Whether the push rule is enabled or not.
    bool enabled = true;
    //! Required. The actions to perform when this rule is matched.
    std::vector<actions::Action> actions;
    //! Required. The ID of this rule.
    std::string rule_id;
    //! The glob-style pattern to match against. Only applicable to content rules.
    std::string pattern;
    //! The conditions that must hold true for an event in order for a rule to be applied to an
    //! event. A rule with no conditions always matches. Only applicable to underride and
    //! override rules.
    std::vector<PushCondition> conditions;

    friend void to_json(nlohmann::json &obj, const PushRule &condition);
    friend void from_json(const nlohmann::json &obj, PushRule &condition);
};

//! All the pushrules to evaluate for events.
struct Ruleset
{
    //! see https://matrix.org/docs/spec/client_server/latest#push-rules
    //
    // A push rule is a single rule that states under what conditions an event should be passed
    // onto a push gateway and how the notification should be presented. There are different
    // "kinds" of push rules and each rule has an associated priority. Every push rule MUST have
    // a kind and rule_id. The rule_id is a unique string within the kind of rule and its'
    // scope: rule_ids do not need to be unique between rules of the same kind on different
    // devices. Rules may have extra keys depending on the value of kind. The different kinds of
    // rule in descending order of priority are:
    std::vector<PushRule> override_;
    std::vector<PushRule> content;
    std::vector<PushRule> room;
    std::vector<PushRule> sender;
    std::vector<PushRule> underride;

    friend void to_json(nlohmann::json &obj, const Ruleset &condition);
    friend void from_json(const nlohmann::json &obj, Ruleset &condition);
};

//! The global ruleset applied to all events.
struct GlobalRuleset
{
    //! The actual ruleset.
    Ruleset global;

    friend void to_json(nlohmann::json &obj, const GlobalRuleset &set);
    friend void from_json(const nlohmann::json &obj, GlobalRuleset &set);
};

//! The response for queries, if a specific ruleset is enabled.
struct Enabled
{
    bool enabled = true;

    friend void to_json(nlohmann::json &obj, const Enabled &enabled);
    friend void from_json(const nlohmann::json &obj, Enabled &enabled);
};
}
}
