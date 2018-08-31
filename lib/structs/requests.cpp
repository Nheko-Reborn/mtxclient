#include "mtx/requests.hpp"

using json = nlohmann::json;

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
to_json(json &obj, const RoomInvite &request)
{
        obj["user_id"] = request.user_id;
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
