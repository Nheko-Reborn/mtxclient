#pragma once

/// @file
/// @brief Server ACL events.

#include <string>
#include <vector>

#if __has_include(<nlohmann/json_fwd.hpp>)
#include <nlohmann/json_fwd.hpp>
#else
#include <nlohmann/json.hpp>
#endif

namespace mtx {
namespace events {
namespace state {

//! Content for the `m.room.server_acl` event.
//
//! An event to indicate which servers are permitted to participate in the room. Server ACLs may
//! allow or deny groups of hosts. All servers participating in the room, including those that are
//! denied, are expected to uphold the server ACL. Servers that do not uphold the ACLs MUST be added
//! to the denied hosts list in order for the ACLs to remain effective.
struct ServerAcl
{
    //! The server names to allow in the room, excluding any port information. Wildcards may be used
    //! to cover a wider range of hosts, where * matches zero or more characters and ? matches
    //! exactly one character.
    //!
    //! This defaults to an empty list when not provided, effectively disallowing every server.
    std::vector<std::string> allow;

    //! The server names to disallow in the room, excluding any port information. Wildcards may be
    //! used to cover a wider range of hosts, where * matches zero or more characters and ? matches
    //! exactly one character.
    //!
    //! This defaults to an empty list when not provided.
    std::vector<std::string> deny;

    //! True to allow server names that are IP address literals. False to deny. Defaults to true if
    //! missing or otherwise not a boolean.
    //!
    //! This is strongly recommended to be set to false as servers running with IP literal names are
    //! strongly discouraged in order to require legitimate homeservers to be backed by a valid
    //! registered domain name.
    bool allow_ip_literals = true;

    //! Deserialization method needed by @p nlohmann::json.
    friend void from_json(const nlohmann::json &obj, ServerAcl &content);

    //! Serialization method needed by @p nlohmann::json.
    friend void to_json(nlohmann::json &obj, const ServerAcl &content);
};

} // namespace state
} // namespace events
} // namespace mtx
