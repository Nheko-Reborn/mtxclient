#include "mtx/requests.hpp"
#include "mtx/events/collections.hpp"
#include "mtx/events/encrypted.hpp"
#include <iostream>

using json = nlohmann::json;
using namespace mtx::events::collections;

namespace mtx {
namespace requests {

std::string
visibilityToString(Visibility visibility)
{
        if (visibility == Visibility::Private) {
                return "private";
        }

        return "public";
}

std::string
presetToString(Preset preset)
{
        switch (preset) {
        case Preset::PrivateChat:
                return "private_chat";
                break;
        case Preset::PublicChat:
                return "public_chat";
                break;
        case Preset::TrustedPrivateChat:
                return "trusted_private_chat";
                break;
        }

        return "private_chat";
}

void
to_json(json &obj, const CreateRoom &request)
{
        if (!request.name.empty())
                obj["name"] = request.name;

        if (!request.topic.empty())
                obj["topic"] = request.topic;

        if (!request.room_alias_name.empty())
                obj["room_alias_name"] = request.room_alias_name;

        if (request.invite.size() != 0)
                obj["invite"] = request.invite;

        obj["is_direct"]  = request.is_direct;
        obj["preset"]     = presetToString(request.preset);
        obj["visibility"] = visibilityToString(request.visibility);
}

void
to_json(json &obj, const Login &request)
{
        if (!request.medium.empty())
                obj["medium"] = request.medium;

        if (!request.address.empty())
                obj["address"] = request.address;

        if (!request.token.empty())
                obj["token"] = request.token;

        if (!request.password.empty())
                obj["password"] = request.password;

        if (!request.device_id.empty())
                obj["device_id"] = request.device_id;

        if (!request.initial_device_display_name.empty())
                obj["initial_device_display_name"] = request.initial_device_display_name;

        obj["user"] = request.user;
        obj["type"] = request.type;
}

void
to_json(json &obj, const ToDeviceMessages &request)
{
        std::map<std::string, std::map<std::string, EventContents>> j_request;
        for (auto it = request.messages.begin(); it != request.messages.end(); it++) {
                std::cout << it->first.to_string() << std::endl;
                for (auto it1 = it->second.begin(); it1 != it->second.begin(); it1++) {
                        std::cout << it1->first << std::endl;
                        std::cout << std::visit([](auto e) { return json(e); }, it1->second).dump(2)
                                  << std::endl;
                        obj["messages"][it->first.to_string()][it1->first] =
                          std::visit([](auto e) { return json(e); }, it1->second);
                }
        }
}

void
to_json(json &obj, const AvatarUrl &request)
{
        obj["avatar_url"] = request.avatar_url;
}

void
to_json(json &obj, const DisplayName &request)
{
        obj["displayname"] = request.displayname;
}

void
to_json(json &obj, const RoomMembershipChange &request)
{
        obj["user_id"] = request.user_id;

        if (!request.reason.empty())
                obj["reason"] = request.reason;
}

void
to_json(json &obj, const TypingNotification &request)
{
        obj["typing"]  = request.typing;
        obj["timeout"] = request.timeout;
}

void
to_json(json &obj, const UploadKeys &request)
{
        obj = json::object();

        if (!request.device_keys.user_id.empty())
                obj["device_keys"] = request.device_keys;

        if (!request.one_time_keys.empty())
                obj["one_time_keys"] = request.one_time_keys;
}

void
to_json(json &obj, const QueryKeys &request)
{
        obj["timeout"]     = request.timeout;
        obj["device_keys"] = request.device_keys;
        obj["token"]       = request.token;
}

} // namespace requests
} // namespace mtx
