#pragma once

#include <nlohmann/json.hpp>

namespace mtx {
namespace responses {

struct JoinedGroups
{
        std::vector<std::string> groups;
};

inline void
from_json(const nlohmann::json &obj, JoinedGroups &res)
{
        res.groups = obj.at("groups").get<std::vector<std::string>>();
}

struct GroupProfile
{
        std::string name;
        std::string avatar_url;
};

inline void
from_json(const nlohmann::json &obj, GroupProfile &res)
{
        if (obj.count("name") != 0 && !obj.at("name").is_null())
                res.name = obj.at("name");
        if (obj.count("avatar_url") != 0 && !obj.at("avatar_url").is_null())
                res.avatar_url = obj.at("avatar_url");
}
}
}
