#pragma once

#include <nlohmann/json.hpp>
#include <string>

using json = nlohmann::json;

namespace mtx {
namespace events {
namespace state {

constexpr uint16_t EventsDefault = 0;
constexpr uint16_t UsersDefault  = 0;
constexpr uint16_t StatesDefault = 50;

constexpr uint16_t User      = 0;
constexpr uint16_t Moderator = 50;
constexpr uint16_t Admin     = 100;

//! Content for the `m.room.power_levels` state event.
//
//! This event specifies the minimum level a user must have in
//! order to perform a certain action. It also specifies the
//! levels of each user in the room.
struct PowerLevels
{
        //! Returns the power_level for a given event type.
        inline uint16_t event_level(const std::string &event_type) const
        {
                if (events.find(event_type) == events.end())
                        return events_default;

                return events.at(event_type);
        }

        //! Returns the power_level for a given event type.
        inline uint16_t state_level(const std::string &event_type) const
        {
                if (events.find(event_type) == events.end())
                        return state_default;

                return events.at(event_type);
        }

        //! Returns the power_level for a given user id.
        inline uint16_t user_level(const std::string &user_id) const
        {
                if (users.find(user_id) == users.end())
                        return users_default;

                return users.at(user_id);
        }

        //! The level required to ban a user. Defaults to **50** if unspecified.
        uint16_t ban = Moderator;
        //! The level required to invite a user.
        //! Defaults to **50** if unspecified.
        uint16_t invite = Moderator;
        //! The level required to kick a user.
        //! Defaults to **50** if unspecified.
        uint16_t kick = Moderator;
        //! The level required to redact an event.
        //! Defaults to **50** if unspecified.
        uint16_t redact = Moderator;
        //! The default level required to send message events.
        //! Defaults to **0** if unspecified.
        uint16_t events_default = User;
        //! The default power level for every user in the room,
        //! unless their user_id is mentioned in the users key.
        //! Defaults to **0** if unspecified.
        uint16_t users_default = User;
        //! The default level required to send state events.
        uint16_t state_default = Moderator;
        //! The level required to send specific event types.
        //! This is a mapping from event type to power level required.
        std::map<std::string, uint16_t> events;
        //! The power levels for specific users.
        //! This is a mapping from user_id to power level for that user.
        std::map<std::string, uint16_t> users;
};

void
from_json(const json &obj, PowerLevels &power_levels);

void
to_json(json &obj, const PowerLevels &power_levels);

} // namespace state
} // namespace events
} // namespace mtx
