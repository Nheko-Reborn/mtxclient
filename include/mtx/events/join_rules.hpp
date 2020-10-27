#pragma once

#if __has_include(<nlohmann/json_fwd.hpp>)
#include <nlohmann/json_fwd.hpp>
#else
#include <nlohmann/json.hpp>
#endif

#include <string>

namespace mtx {
namespace events {
namespace state {

enum class JoinRule
{
        //! Anyone can join the room without any prior action.
        Public,
        //! A user who wishes to join the room must first receive
        //! an invite to the room from someone already inside of the room.
        Invite,
        //! Reserved but not yet implemented by the Matrix specification.
        Knock,
        //! Reserved but not yet implemented by the Matrix specification.
        Private,
};

std::string
joinRuleToString(const JoinRule &rule);

JoinRule
stringToJoinRule(const std::string &rule);

//! Content of the `m.room.join_rules` state event.
struct JoinRules
{
        //! The type of rules used for users wishing to join this room.
        JoinRule join_rule;
};

void
from_json(const nlohmann::json &obj, JoinRules &join_rules);

void
to_json(nlohmann::json &obj, const JoinRules &join_rules);

} // namespace state
} // namespace events
} // namespace mtx
