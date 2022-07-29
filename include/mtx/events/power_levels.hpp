#pragma once

/// @file
/// @brief The state event governing the different permissions in a room.

#if __has_include(<nlohmann/json_fwd.hpp>)
#include <nlohmann/json_fwd.hpp>
#else
#include <nlohmann/json.hpp>
#endif

#include <string>
#include <string_view>

namespace mtx {
namespace events {
namespace state {
//! The type representing a power level.
using power_level_t = int64_t;

//! The default level required for events
constexpr power_level_t EventsDefault = 0;
//! The default level for users.
constexpr power_level_t UsersDefault = 0;
//! The default level required for state events.
constexpr power_level_t StatesDefault = 50;

//! The power level usually associated with normal users.
constexpr power_level_t User = 0;
//! The power level usually associated with moderators.
constexpr power_level_t Moderator = 50;
//! The power level usually associated with admins.
constexpr power_level_t Admin = 100;

//! different predefined keys for notification levels
namespace notification_keys {
//! The level required to trigger an @room notification. Defaults to 50 if unspecified.
constexpr std::string_view room = "room";
}

/// @brief Content for the `m.room.power_levels` state event.
///
/// This event specifies the minimum level a user must have in
/// order to perform a certain action. It also specifies the
/// levels of each user in the room.
struct PowerLevels
{
    //! Returns the power_level for a given event type.
    inline power_level_t event_level(const std::string &event_type) const
    {
        if (events.find(event_type) == events.end())
            return events_default;

        return events.at(event_type);
    }

    //! Returns the power_level for a given event type.
    inline power_level_t state_level(const std::string &event_type) const
    {
        if (events.find(event_type) == events.end())
            return state_default;

        return events.at(event_type);
    }

    //! Returns the power_level for a given user id.
    inline power_level_t user_level(const std::string &user_id) const
    {
        if (users.find(user_id) == users.end())
            return users_default;

        return users.at(user_id);
    }

    inline power_level_t notification_level(std::string_view notification_key) const
    {
        if (auto it = notifications.find(notification_key); it != notifications.end())
            return it->second;
        else if (notification_key == notification_keys::room)
            return 50;
        else // spec doesn't actually specify that?
            return 50;
    }

    //! The level required to ban a user. Defaults to **50** if unspecified.
    power_level_t ban = Moderator;
    //! The level required to invite a user.
    //! Defaults to **50** if unspecified.
    power_level_t invite = Moderator;
    //! The level required to kick a user.
    //! Defaults to **50** if unspecified.
    power_level_t kick = Moderator;
    //! The level required to redact an event.
    //! Defaults to **50** if unspecified.
    power_level_t redact = Moderator;
    //! The default level required to send message events.
    //! Defaults to **0** if unspecified.
    power_level_t events_default = User;
    //! The default power level for every user in the room,
    //! unless their user_id is mentioned in the users key.
    //! Defaults to **0** if unspecified.
    power_level_t users_default = User;
    //! The default level required to send state events.
    power_level_t state_default = Moderator;
    //! The level required to send specific event types.
    //! This is a mapping from event type to power level required.
    std::map<std::string, power_level_t, std::less<>> events;
    //! The power levels for specific users.
    //! This is a mapping from user_id to power level for that user.
    std::map<std::string, power_level_t, std::less<>> users;
    //! The power level requirements for specific notification types. This is a mapping from key
    //! to power level for that notifications key.
    std::map<std::string, power_level_t, std::less<>> notifications;

    friend void from_json(const nlohmann::json &obj, PowerLevels &power_levels);
    friend void to_json(nlohmann::json &obj, const PowerLevels &power_levels);
};

} // namespace state
} // namespace events
} // namespace mtx
