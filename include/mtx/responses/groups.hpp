#pragma once

/// @file
/// @brief Groups related endpoints.

#if __has_include(<nlohmann/json_fwd.hpp>)
#include <nlohmann/json_fwd.hpp>
#else
#include <nlohmann/json.hpp>
#endif

namespace mtx {
namespace responses {
//! The list of joined groups.
struct JoinedGroups
{
    //! joined group ids.
    std::vector<std::string> groups;
};

void
from_json(const nlohmann::json &obj, JoinedGroups &res);

//! The profile of a group.
struct GroupProfile
{
    //! The group name.
    std::string name;
    //! The group avatar.
    std::string avatar_url;
};

void
from_json(const nlohmann::json &obj, GroupProfile &res);
}
}
