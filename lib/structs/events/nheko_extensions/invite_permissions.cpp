#include "mtx/events/nheko_extensions/invite_permissions.hpp"

#include <tuple>

#include <nlohmann/json.hpp>

#include <mtx/log.hpp>

static std::tuple<std::map<std::string, std::string, std::less<>>,
                  std::map<std::string, std::string, std::less<>>>
parse_subset(const std::string &key, const nlohmann::json &obj)
{
    std::map<std::string, std::string, std::less<>> allow, deny;

    if (auto o = obj.find(key); o != obj.end()) {
        auto a = o->value("allow", std::map<std::string, nlohmann::json>());
        auto d = o->value("deny", std::map<std::string, nlohmann::json>());

        for (const auto &[k, v] : a) {
            allow.emplace(k, v.dump());
        }
        for (const auto &[k, v] : d) {
            deny.emplace(k, v.dump());
        }
    }

    return {allow, deny};
}

static void
emit_subset(nlohmann::json &obj,
            std::string_view key,
            const std::map<std::string, std::string, std::less<>> &allow,
            const std::map<std::string, std::string, std::less<>> &deny)
{
    for (const auto &[k, v] : allow) {
        obj[key]["allow"][k] = nlohmann::json::parse(v);
    }
    for (const auto &[k, v] : deny) {
        obj[key]["deny"][k] = nlohmann::json::parse(v);
    }
}

namespace mtx {
namespace events {
namespace account_data {
namespace nheko_extensions {

void
from_json(const nlohmann::json &obj, InvitePermissions &content)
{
    if (obj.contains("default")) {
        content.default_ = obj.at("default").get<std::string>();
    } else {
        content.default_ = "allow";
    }

    std::tie(content.server_allow, content.server_deny) = parse_subset("server", obj);
    std::tie(content.room_allow, content.room_deny)     = parse_subset("room", obj);
    std::tie(content.user_allow, content.user_deny)     = parse_subset("user", obj);
}

void
to_json(nlohmann::json &obj, const InvitePermissions &content)
{
    obj["default"] = content.default_;

    emit_subset(obj, "server", content.server_allow, content.server_deny);
    emit_subset(obj, "room", content.room_allow, content.room_deny);
    emit_subset(obj, "user", content.user_allow, content.user_deny);
}

bool
InvitePermissions::invite_allowed(std::string_view room_id, std::string_view inviter) const
{
    if (this->user_deny.contains(inviter))
        return false;
    if (this->user_allow.contains(inviter))
        return true;
    if (this->room_deny.contains(room_id))
        return false;
    if (this->room_allow.contains(room_id))
        return true;

    if (auto pos = inviter.find_first_of(':'); pos != std::string_view::npos) {
        auto server = inviter.substr(pos + 1);
        if (this->server_deny.contains(server))
            return false;
        if (this->server_allow.contains(server))
            return false;
    }

    return this->default_ != "deny";
}
}
} // namespace account_data
} // namespace events
} // namespace mtx
