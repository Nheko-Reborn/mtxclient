#pragma once

#if __has_include(<nlohmann/json_fwd.hpp>)
#include <nlohmann/json_fwd.hpp>
#else
#include <nlohmann/json.hpp>
#endif

namespace mtx {
namespace responses {

struct JoinedGroups
{
        std::vector<std::string> groups;
};

void
from_json(const nlohmann::json &obj, JoinedGroups &res);

struct GroupProfile
{
        std::string name;
        std::string avatar_url;
};

void
from_json(const nlohmann::json &obj, GroupProfile &res);
}
}
