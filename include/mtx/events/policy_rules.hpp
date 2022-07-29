#pragma once

/// @file
/// @brief policy rules for room moderation
///
/// With Matrix being an open network where anyone can participate, a very wide range of content
/// exists and it is important that users are empowered to select which content they wish to see,
/// and which content they wish to block. By extension, room moderators and server admins should
/// also be able to select which content they do not wish to host in their rooms and servers.
///
/// The protocol’s position on this is one of neutrality: it should not be deciding what content is
/// undesirable for any particular entity and should instead be empowering those entities to make
/// their own decisions. As such, a generic framework for communicating “moderation policy lists” or
/// “moderation policy rooms” is described. Note that this module only describes the data structures
/// and not how they should be interpreting: the entity making the decisions on filtering is best
/// positioned to interpret the rules how it sees fit.
///
/// Moderation policy lists are stored as room state events. There are no restrictions on how the
/// rooms can be configured (they could be public, private, encrypted, etc).
///
/// There are currently 3 kinds of entities which can be affected by rules: user, server, and room.
/// All 3 are described with m.policy.rule.<kind> state events. The state_key for a policy rule is
/// an arbitrary string decided by the sender of the rule.
///
/// Rules contain recommendations and reasons for the rule existing. The reason is a human-readable
/// string which describes the recommendation. Currently only one recommendation, m.ban, is
/// specified.

#include <optional>
#include <string_view>

#if __has_include(<nlohmann/json_fwd.hpp>)
#include <nlohmann/json_fwd.hpp>
#else
#include <nlohmann/json.hpp>
#endif

namespace mtx {
namespace events {
namespace state {
namespace policy_rule {

//! A recommendation of how to handle the entity.
namespace recommendation {
/// @brief a recommendation to ban an entity
///
/// When this recommendation is used, the entities affected by the rule should be banned from
/// participation where possible. The enforcement of this is deliberately left as an implementation
/// detail to avoid the protocol imposing its opinion on how the policy list is to be interpreted.
constexpr std::string_view ban = "m.ban";
}

//! Content of the `m.policy.rule.*` events.
//
//! The entity described by the state events can contain * and ? to match zero or more and one or
//! more characters respectively. Note that rules against rooms can describe a room ID or room alias
//! - the subscriber is responsible for resolving the alias to a room ID if desired.
struct Rule
{
    //! Required: The entity affected by this rule. Glob characters * and ? can be used to match
    //! zero or more and one or more characters respectively.
    std::string entity;

    //! Required: The human-readable description for the recommendation.
    std::string reason;

    //! Required: The suggested action to take. Currently only m.ban is specified.
    std::string recommendation;

    friend void from_json(const nlohmann::json &obj, Rule &create);
    friend void to_json(nlohmann::json &obj, const Rule &create);
};

struct UserRule : public Rule
{
    friend void from_json(const nlohmann::json &obj, UserRule &create);
    friend void to_json(nlohmann::json &obj, const UserRule &create);
};
struct RoomRule : public Rule
{
    friend void from_json(const nlohmann::json &obj, RoomRule &create);
    friend void to_json(nlohmann::json &obj, const RoomRule &create);
};
struct ServerRule : public Rule
{
    friend void from_json(const nlohmann::json &obj, ServerRule &create);
    friend void to_json(nlohmann::json &obj, const ServerRule &create);
};
} // namespace policy_rule
} // namespace state
} // namespace events
} // namespace mtx
