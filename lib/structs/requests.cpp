#include "mtx/requests.hpp"
#include "mtx/events/collections.hpp"
#include "mtx/events/encrypted.hpp"
#include <iostream>
#include <nlohmann/json.hpp>

using json = nlohmann::json;
using namespace mtx::events::collections;

namespace mtx {
namespace requests {

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
to_json(json &obj, const PublicRoomVisibility &request)
{
        obj["visibility"] = mtx::common::visibilityToString(request.visibility);
}

void
to_json(json &obj, const PublicRoomsFilter &request)
{
        obj["generic_search_term"] = request.generic_search_term;
}

void
to_json(json &obj, const PublicRooms &request)
{
        if (request.limit > 0) {
                obj["limit"] = request.limit;
        }

        if (!request.since.empty()) {
                obj["since"] = request.since;
        }

        if (!request.filter.generic_search_term.empty()) {
                obj["filter"] = request.filter;
        }

        // Based on the spec, third_party_instance_id can only be used if
        // include_all_networks is false. A case where the latter is true and
        // the former is set is invalid.
        if (request.include_all_networks && !request.third_party_instance_id.empty()) {
                throw std::invalid_argument(
                  "third_party_instance_id can only be set if include_all_networks is false");
        } else if (!request.third_party_instance_id.empty()) {
                obj["third_party_instance_id"] = request.third_party_instance_id;
                obj["include_all_networks"]    = false;
        } else {
                obj["include_all_networks"] = true;
        }
}

void
to_json(json &obj, const SignedOneTimeKey &request)
{
        obj["key"]        = request.key;
        obj["signatures"] = request.signatures;
}

void
to_json(json &obj, const UploadKeys &request)
{
        obj = json::object();

        if (!request.device_keys.user_id.empty())
                obj["device_keys"] = request.device_keys;

        for (const auto &[key_id, key] : request.one_time_keys) {
                obj["one_time_keys"][key_id] =
                  std::visit([](const auto &e) { return json(e); }, key);
        }
}

void
to_json(json &obj, const ClaimKeys &request)
{
        obj["timeout"]       = request.timeout;
        obj["one_time_keys"] = request.one_time_keys;
}

void
to_json(json &obj, const QueryKeys &request)
{
        obj["timeout"]     = request.timeout;
        obj["device_keys"] = request.device_keys;
        obj["token"]       = request.token;
}

void
to_json(json &obj, const KeySignaturesUpload &req)
{
        for (const auto &[user_id, idToKey] : req.signatures)
                for (const auto &[key_id, keyVar] : idToKey)
                        obj[user_id][key_id] =
                          std::visit([](const auto &e) { return json(e); }, keyVar);
}


void
to_json(json &obj, const PusherData &data)
{
        if (!data.url.empty()) {
                obj["url"] = data.url;
        }
        if (!data.format.empty()) {
                obj["format"] = data.format;
        }
}

void
to_json(json &obj, const SetPusher &req)
{
        obj["pushkey"] = req.pushkey;
        obj["kind"] = req.kind;
        obj["app_id"] = req.app_id;
        obj["app_display_name"] = req.app_display_name;
        obj["device_display_name"] = req.device_display_name;
        if (!req.profile_tag.empty()) {
                obj["profile_tag"] = req.profile_tag;
        }
        obj["lang"] = req.lang;
        obj["data"] = req.data;
        obj["append"] = req.append;
}

} // namespace requests
} // namespace mtx
