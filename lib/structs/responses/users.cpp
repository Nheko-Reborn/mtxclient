#include "mtx/responses/users.hpp"

#include <nlohmann/json.hpp>

using json = nlohmann::json;

namespace mtx {
namespace responses {

void
from_json(const json &obj, User &user)
{
    if (obj.count("avatar_url") != 0 && !obj.at("avatar_url").is_null())
        user.avatar_url = obj.at("avatar_url").get<std::string>();

    if (obj.count("display_name") != 0 && !obj.at("display_name").is_null())
        user.display_name = obj.at("display_name").get<std::string>();

    user.user_id = obj.at("user_id").get<std::string>();
}

void
from_json(const json &obj, Users &users)
{
    users.limited = obj.at("limited").get<bool>();
    users.results = obj.at("results").get<std::vector<User>>();
}
}
}
