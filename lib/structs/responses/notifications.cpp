#include "mtx/responses/notifications.hpp"
#include "mtx/responses/common.hpp"

using json = nlohmann::json;

namespace mtx {
namespace responses {

void
from_json(const json &obj, Notification &res)
{
        res.actions = obj.at("actions");
        res.read    = obj.at("read");
        res.room_id = obj.at("room_id");
        res.ts      = obj.at("ts");

        if (!obj.at("profile_tag").is_null())
                res.profile_tag = obj.at("profile_tag");

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
from_json(const json &obj, Notifications &res)
{
        // res.next_token    = obj.at("next_token").get<std::string>();
        res.notifications = obj.at("notifications").get<std::vector<Notification>>();
}
} // namespace responses
} // namespace mtx
