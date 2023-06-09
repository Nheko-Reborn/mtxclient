#include <nlohmann/json.hpp>
#include <unordered_map>

#include "mtx/events/account_data/ignored_users.hpp"

namespace mtx {
namespace events {
namespace account_data {

void
from_json(const nlohmann::json &obj, IgnoredUsers &content)
{
    const auto map = obj.at("ignored_users").get<std::unordered_map<std::string, nlohmann::json>>();
    for (const auto &[key, value] : map) {
        data::IgnoredUser user;
        user.id = key;
        content.users.push_back(user);
    }
}

void
to_json(nlohmann::json &obj, const IgnoredUsers &content)
{
    std::unordered_map<std::string, nlohmann::json> map;
    for (const data::IgnoredUser &user : content.users) {
        map[user.id] = {};
    }
    obj["ignored_users"] = map;
}

} // namespace account_data
} // namespace events
} // namespace mtx