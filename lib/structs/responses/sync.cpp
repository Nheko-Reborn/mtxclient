#include "mtx/responses/sync.hpp"
#include "mtx/events/collections.hpp"
#include "mtx/log.hpp"
#include "mtx/responses/common.hpp"

#include <nlohmann/json.hpp>

#include <variant>

using json = nlohmann::json;

namespace mtx {
namespace responses {

void
from_json(const json &obj, AccountData &account_data)
{
    utils::parse_room_account_data_events(obj.at("events"), account_data.events);
}

void
from_json(const json &obj, State &state)
{
    utils::parse_state_events(obj.at("events"), state.events);
}

void
from_json(const json &obj, Timeline &timeline)
{
    timeline.prev_batch = obj.value("prev_batch", std::string{});
    timeline.limited    = obj.value("limited", false);

    utils::parse_timeline_events(obj.at("events"), timeline.events);
}

void
from_json(const json &obj, UnreadNotifications &notifications)
{
    if (obj.find("highlight_count") != obj.end())
        notifications.highlight_count = obj.at("highlight_count");

    if (obj.find("notification_count") != obj.end())
        notifications.notification_count = obj.at("notification_count");
}

void
from_json(const json &obj, Ephemeral &ephemeral)
{
    if (obj.count("events") == 0)
        return;

    utils::parse_ephemeral_events(obj.at("events"), ephemeral.events);
}

void
from_json(const json &obj, JoinedRoom &room)
{
    if (obj.count("state") != 0)
        room.state = obj.at("state").get<State>();

    if (obj.count("timeline") != 0)
        room.timeline = obj.at("timeline").get<Timeline>();

    if (obj.find("unread_notifications") != obj.end())
        room.unread_notifications = obj.at("unread_notifications").get<UnreadNotifications>();

    if (obj.find("ephemeral") != obj.end())
        room.ephemeral = obj.at("ephemeral").get<Ephemeral>();

    if (obj.count("account_data") != 0) {
        if (obj.at("account_data").count("events") != 0)
            room.account_data = obj.at("account_data").get<AccountData>();
    }
}

void
from_json(const json &obj, LeftRoom &room)
{
    if (obj.count("state") != 0)
        room.state = obj.at("state").get<State>();

    if (obj.count("timeline") != 0)
        room.timeline = obj.at("timeline").get<Timeline>();
}

std::string
InvitedRoom::name() const
{
    using Name   = mtx::events::StrippedEvent<mtx::events::state::Name>;
    using Member = mtx::events::StrippedEvent<mtx::events::state::Member>;

    std::string room_name;
    std::string member_name;

    for (const auto &event : invite_state) {
        if (auto name = std::get_if<Name>(&event); name != nullptr) {
            room_name = name->content.name;
        } else if (auto avatar = std::get_if<Member>(&event); avatar != nullptr) {
            if (member_name.empty())
                member_name = avatar->content.display_name;
        }
    }

    if (room_name.empty())
        return member_name;

    return room_name;
}

std::string
InvitedRoom::avatar() const
{
    using Avatar = mtx::events::StrippedEvent<mtx::events::state::Avatar>;
    using Member = mtx::events::StrippedEvent<mtx::events::state::Member>;

    std::string room_avatar;
    std::string member_avatar;

    for (const auto &event : invite_state) {
        if (auto avatar = std::get_if<Avatar>(&event); avatar != nullptr) {
            room_avatar = avatar->content.url;
        } else if (auto member = std::get_if<Member>(&event); member != nullptr) {
            // Pick the first avatar.
            if (member_avatar.empty())
                member_avatar = member->content.avatar_url;
        }
    }

    if (room_avatar.empty())
        return member_avatar;

    return room_avatar;
}

void
from_json(const json &obj, InvitedRoom &room)
{
    utils::parse_stripped_events(obj.at("invite_state").at("events"), room.invite_state);
}

void
from_json(const json &obj, Rooms &rooms)
{
    if (obj.count("join") != 0) {
        rooms.join = obj.at("join").get<std::map<std::string, JoinedRoom>>();
    }

    if (obj.count("leave") != 0) {
        rooms.leave = obj.at("leave").get<std::map<std::string, LeftRoom>>();
    }

    if (obj.count("invite") != 0) {
        rooms.invite = obj.at("invite").get<std::map<std::string, InvitedRoom>>();
    }
}

void
from_json(const json &obj, DeviceLists &device_lists)
{
    if (obj.count("changed") != 0)
        device_lists.changed = obj.at("changed").get<std::vector<std::string>>();

    if (obj.count("left") != 0)
        device_lists.left = obj.at("left").get<std::vector<std::string>>();
}

void
from_json(const json &obj, ToDevice &to_device)
{
    if (obj.count("events") != 0)
        utils::parse_device_events(obj.at("events"), to_device.events);
}

void
from_json(const json &obj, Sync &response)
{
    if (obj.count("rooms") != 0)
        response.rooms = obj.at("rooms").get<Rooms>();

    if (obj.count("device_lists") != 0)
        response.device_lists = obj.at("device_lists").get<DeviceLists>();

    if (obj.count("to_device") != 0) {
        response.to_device = obj.at("to_device").get<ToDevice>();
    }

    if (obj.count("device_one_time_keys_count") != 0)
        response.device_one_time_keys_count =
          obj.at("device_one_time_keys_count").get<std::map<std::string, uint16_t>>();

    if (obj.count("presence") != 0 && obj.at("presence").contains("events")) {
        response.presence =
          obj.at("presence")
            .at("events")
            .get<std::vector<mtx::events::Event<mtx::events::presence::Presence>>>();
    }

    if (obj.count("account_data") != 0) {
        if (obj.at("account_data").count("events") != 0)
            response.account_data = obj.at("account_data").get<AccountData>();
    }

    response.next_batch = obj.at("next_batch").get<std::string>();
}
}
}
