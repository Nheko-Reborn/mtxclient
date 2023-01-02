#include "mtx/responses/sync.hpp"
#include "mtx/events/collections.hpp"
#include "mtx/log.hpp"
#include "mtx/responses/common.hpp"

#include <nlohmann/json.hpp>

#include <algorithm>
#include <variant>

using json = nlohmann::json;

namespace mtx {
namespace responses {

void
from_json(const json &obj, AccountData &account_data)
{
    if (auto it = obj.find("events"); it != obj.end() && it->is_array())
        utils::parse_room_account_data_events(*it, account_data.events);
}

void
from_json(const json &obj, State &state)
{
    if (auto it = obj.find("events"); it != obj.end() && it->is_array())
        utils::parse_state_events(*it, state.events);
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
    if (auto it = obj.find("highlight_count"); it != obj.end())
        notifications.highlight_count = it->get<uint64_t>();

    if (auto it = obj.find("notification_count"); it != obj.end())
        notifications.notification_count = it->get<uint64_t>();
}

void
from_json(const json &obj, Ephemeral &ephemeral)
{
    if (auto it = obj.find("events"); it != obj.end() && it->is_array())
        utils::parse_ephemeral_events(*it, ephemeral.events);
}

void
from_json(const json &obj, JoinedRoom &room)
{
    if (auto it = obj.find("state"); it != obj.end())
        room.state = it->get<State>();

    if (auto it = obj.find("timeline"); it != obj.end())
        room.timeline = it->get<Timeline>();

    if (auto it = obj.find("unread_notifications"); it != obj.end())
        room.unread_notifications = it->get<UnreadNotifications>();

    if (auto it = obj.find("ephemeral"); it != obj.end())
        room.ephemeral = it->get<Ephemeral>();

    if (auto it = obj.find("account_data"); it != obj.end())
        room.account_data = it->get<AccountData>();
}

void
from_json(const json &obj, LeftRoom &room)
{
    if (auto it = obj.find("state"); it != obj.end())
        room.state = it->get<State>();

    if (auto it = obj.find("timeline"); it != obj.end())
        room.timeline = it->get<Timeline>();
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
    if (auto state = obj.find("invite_state"); state != obj.end())
        if (auto events = state->find("events"); events != state->end())
            utils::parse_stripped_events(*events, room.invite_state);
}

void
from_json(const json &obj, KnockedRoom &room)
{
    if (auto state = obj.find("knock_state"); state != obj.end())
        if (auto events = state->find("events"); events != state->end())
            utils::parse_stripped_events(*events, room.knock_state);
}

void
from_json(const json &obj, Rooms &rooms)
{
    if (auto entries = obj.find("join"); entries != obj.end()) {
        for (const auto &r : entries->items()) {
            if (r.key().size() < 256) {
                rooms.join.emplace_hint(rooms.join.end(), r.key(), r.value().get<JoinedRoom>());
            } else {
                mtx::utils::log::log()->warn("Skipping roomid which exceeds 255 bytes.");
            }
        }
    }

    if (auto entries = obj.find("leave"); entries != obj.end()) {
        for (const auto &r : entries->items()) {
            if (r.key().size() < 256) {
                rooms.leave.emplace_hint(rooms.leave.end(), r.key(), r.value().get<LeftRoom>());
            } else {
                mtx::utils::log::log()->warn("Skipping roomid which exceeds 255 bytes.");
            }
        }
    }

    if (auto entries = obj.find("invite"); entries != obj.end()) {
        for (const auto &r : entries->items()) {
            if (r.key().size() < 256) {
                rooms.invite.emplace_hint(
                  rooms.invite.end(), r.key(), r.value().get<InvitedRoom>());
            } else {
                mtx::utils::log::log()->warn("Skipping roomid which exceeds 255 bytes.");
            }
        }
    }

    if (auto entries = obj.find("knock"); entries != obj.end()) {
        for (const auto &r : entries->items()) {
            if (r.key().size() < 256) {
                rooms.knock.emplace_hint(rooms.knock.end(), r.key(), r.value().get<KnockedRoom>());
            } else {
                mtx::utils::log::log()->warn("Skipping roomid which exceeds 255 bytes.");
            }
        }
    }
}

void
from_json(const json &obj, DeviceLists &device_lists)
{
    if (obj.count("changed") != 0) {
        device_lists.changed = obj.at("changed").get<std::vector<std::string>>();

        std::erase_if(device_lists.changed, [](const std::string &user) {
            if (user.size() > 255) {
                mtx::utils::log::log()->warn("Invalid userid in device list changed.");
                return true;
            } else
                return false;
        });
    }

    if (obj.count("left") != 0) {
        device_lists.left = obj.at("left").get<std::vector<std::string>>();

        std::erase_if(device_lists.left, [](const std::string &user) {
            if (user.size() > 255) {
                mtx::utils::log::log()->warn("Invalid userid in device list left.");
                return true;
            } else
                return false;
        });
    }
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
    if (auto it = obj.find("rooms"); it != obj.end())
        response.rooms = it->get<Rooms>();

    if (auto it = obj.find("device_lists"); it != obj.end())
        response.device_lists = it->get<DeviceLists>();

    if (auto it = obj.find("to_device"); it != obj.end())
        response.to_device = it->get<ToDevice>();

    if (auto it = obj.find("device_one_time_keys_count"); it != obj.end())
        response.device_one_time_keys_count = it->get<std::map<std::string, uint16_t>>();

    if (auto fallback_keys = obj.find("device_unused_fallback_key_types");
        fallback_keys != obj.end() && fallback_keys->is_array())
        response.device_unused_fallback_key_types = fallback_keys->get<std::vector<std::string>>();

    if (obj.count("presence") != 0 && obj.at("presence").contains("events")) {
        const auto &events = obj.at("presence").at("events");
        response.presence.reserve(events.size());
        for (const auto &e : events) {
            try {
                response.presence.push_back(
                  e.get<mtx::events::Event<mtx::events::presence::Presence>>());
            } catch (std::exception &ex) {
                mtx::utils::log::log()->warn(
                  "Error parsing presence event: {}, {}", ex.what(), e.dump(2));
            }
        }
    }

    if (auto it = obj.find("account_data"); it != obj.end())
        response.account_data = it->get<AccountData>();

    response.next_batch = obj.at("next_batch").get<std::string>();
}
}
}
