#pragma once

/// @file
/// @brief Pushrules and notification settings.

#if __has_include(<nlohmann/json_fwd.hpp>)
#include <nlohmann/json_fwd.hpp>
#else
#include <nlohmann/json.hpp>
#endif

#include <compare>
#include <string>
#include <variant>
#include <vector>

#include "mtx/events/common.hpp"
#include "mtx/events/power_levels.hpp"

namespace mtx {
namespace events {
namespace collections {
struct TimelineEvent;
}
}

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

    //! The relation type to match on. Only valid for `im.nheko.msc3664.related_event_match`
    //! conditions.
    mtx::common::RelationType rel_type = mtx::common::RelationType::Unsupported;
    //! Wether to match fallback relations or not.
    bool include_fallback = false;

    friend void to_json(nlohmann::json &obj, const PushCondition &condition);
    friend void from_json(const nlohmann::json &obj, PushCondition &condition);
};
//! Namespace for the different push actions.
namespace actions {
//! Notify the user.
struct notify
{
    bool operator==(const notify &) const noexcept = default;
};
//! Don't notify the user.
struct dont_notify
{
    bool operator==(const dont_notify &) const noexcept = default;
};
/// @brief This enables notifications for matching events but activates homeserver specific
/// behaviour to intelligently coalesce multiple events into a single notification.
///
/// Not all homeservers may support this. Those that do not support it should treat it as the notify
/// action.
struct coalesce
{
    bool operator==(const coalesce &) const noexcept = default;
};
//! Play a sound.
struct set_tweak_sound
{
    //! The sound to play.
    std::string value = "default";

    bool operator==(const set_tweak_sound &) const noexcept = default;
};
//! Highlight the message.
struct set_tweak_highlight
{
    bool value = true;

    bool operator==(const set_tweak_highlight &) const noexcept = default;
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

    bool operator==(const Actions &) const noexcept = default;
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

//! An optimized structure to calculate notifications for events.
///
/// You will want to cache this for as long as possible (until the pushrules change), since
/// constructing this is somewhat expensive.
class PushRuleEvaluator
{
public:
    //! Construct a new push evaluator. Pass the current set of pushrules to evaluate.
    PushRuleEvaluator(const Ruleset &rules);
    ~PushRuleEvaluator();

    //! Additional room information needed to evaluate push rules.
    struct RoomContext
    {
        //! The displayname of the user in the room.
        std::string user_display_name;
        //! the membercount of the room
        std::size_t member_count = 0;
        //! The powerlevels event in this room
        mtx::events::state::PowerLevels power_levels;
    };

    //! Evaluate the pushrules for @event .
    ///
    /// You need to have the room_id set for the event.
    /// `relatedEvents` is a mapping of rel_type to event. Pass all the events that are related to
    /// by this event here.
    /// \returns the actions to apply.
    [[nodiscard]] std::vector<actions::Action> evaluate(
      const mtx::events::collections::TimelineEvent &event,
      const RoomContext &ctx,
      const std::vector<std::pair<mtx::common::Relation, mtx::events::collections::TimelineEvent>>
        &relatedEvents) const;

private:
    struct OptimizedRules;
    std::unique_ptr<OptimizedRules> rules;
};
}
}
