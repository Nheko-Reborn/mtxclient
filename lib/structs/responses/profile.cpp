#include <string>

#include "mtx/responses/profile.hpp"

using json = nlohmann::json;

namespace mtx {
namespace responses {

void
from_json(const json &obj, Profile &profile)
{
        if (obj.count("avatar_url") != 0 && !obj.at("avatar_url").is_null())
                profile.avatar_url = obj.at("avatar_url").get<std::string>();

        if (obj.count("displayname") != 0 && !obj.at("displayname").is_null())
                profile.display_name = obj.at("displayname").get<std::string>();
}

void
from_json(const json &obj, AvatarUrl &avatar)
{
        if (obj.count("avatar_url") != 0 && !obj.at("avatar_url").is_null())
                avatar.avatar_url = obj.at("avatar_url").get<std::string>();
}
}
}
