#include "mtx/responses/notifications.hpp"
#include "mtx/responses/common.hpp"

#include <nlohmann/json.hpp>

using json = nlohmann::json;

namespace mtx {
namespace responses {

void
from_json(const json &obj, Notification &res)
{
    res.actions = obj.at("actions").get<decltype(res.actions)>();
    res.read    = obj.at("read").get<bool>();
    res.room_id = obj.at("room_id").get<std::string>();
    res.ts      = obj.at("ts").get<uint64_t>();

    if (obj.find("profile_tag") != obj.end() && !obj.at("profile_tag").is_null())
        res.profile_tag = obj.at("profile_tag").get<std::string>();

    // HACK to work around the fact that there isn't
    // a method to parse a timeline event from a json object.
    //
    // TODO: Create method that retrieves a TimelineEvents variant from a json object.
    // Ideally with an optional type to indicate failure.
    std::vector<events::collections::TimelineEvents> tmp;
    tmp.reserve(1);

    json arr;
    arr.push_back(obj.at("event"));

    utils::parse_timeline_events(arr, tmp);

    if (!tmp.empty())
        res.event = tmp.at(0);
}

void
to_json(json &obj, const Notification &res)
{
    obj["actions"] = res.actions;
    obj["read"]    = res.read;
    obj["room_id"] = res.room_id;
    obj["ts"]      = res.ts;

    // HACK to work around the fact that there isn't
    // a method to parse a timeline event from a json object.
    //
    // TODO: Create method that retrieves a TimelineEvents variant from a json object.
    // Ideally with an optional type to indicate failure.
    std::vector<events::collections::TimelineEvents> tmp;
    tmp.reserve(1);

    json arr;
    tmp.push_back(res.event);

    utils::compose_timeline_events(arr, tmp);

    if (!tmp.empty()) {
        obj["event"] = arr;
    }

    if (!res.profile_tag.empty()) {
        obj["profile_tag"] = res.profile_tag;
    }
}

void
from_json(const json &obj, Notifications &res)
{
    // res.next_token    = obj.at("next_token").get<std::string>();
    res.notifications = obj.at("notifications").get<std::vector<Notification>>();
}

void
to_json(json &obj, const Notifications &notif)
{
    obj["notifications"] = notif.notifications;
}
} // namespace responses
} // namespace mtx
