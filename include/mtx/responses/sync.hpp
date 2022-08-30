#pragma once

/// @file
/// @brief Response from the /sync API.

#include <map>
#include <string>
#include <vector>

#include "mtx/events/collections.hpp"

#if __has_include(<nlohmann/json_fwd.hpp>)
#include <nlohmann/json_fwd.hpp>
#else
#include <nlohmann/json.hpp>
#endif

namespace mtx {
namespace responses {

//! Room specific Account Data events.
struct AccountData
{
    //! List of events.
    std::vector<events::collections::RoomAccountDataEvents> events;

    friend void from_json(const nlohmann::json &obj, AccountData &account_data);
};

//! State events.
struct State
{
    //! List of events.
    std::vector<events::collections::StateEvents> events;

    friend void from_json(const nlohmann::json &obj, State &state);
};

//! State and Room events.
struct Timeline
{
    //! List of events.
    std::vector<events::collections::TimelineEvents> events;
    //! A token that can be supplied to to the from parameter of
    //! the rooms/{roomId}/messages endpoint.
    std::string prev_batch;
    //! **true** if the number of events returned was limited by the
    //! limit on the filter.
    bool limited = false;

    friend void from_json(const nlohmann::json &obj, Timeline &timeline);
};

//! Counts of unread notifications for this room
struct UnreadNotifications
{
    //! The number of unread notifications for this room with the
    //! highlight flag set.
    uint64_t highlight_count = 0;
    //! The total number of unread notifications for this room.
    uint64_t notification_count = 0;

    friend void from_json(const nlohmann::json &obj, UnreadNotifications &notifications);
};

//! The ephemeral events in the room that aren't recorded in
//! the timeline or state of the room. e.g. typing.
struct Ephemeral
{
    //! A list of ephemeral events like typing and read notifications.
    std::vector<events::collections::EphemeralEvents> events;

    friend void from_json(const nlohmann::json &obj, Ephemeral &ephemeral);
};

//! A room that the user has joined.
struct JoinedRoom
{
    //! Updates to the state, between the time indicated by the since parameter,
    //! and the start of the timeline (or all state up to the start of the timeline,
    //! if since is not given, or full_state is true)
    State state;
    //! The timeline of messages and state changes in the room.
    Timeline timeline;
    //! Counts of unread notifications for this room.
    UnreadNotifications unread_notifications;
    //! The ephemeral events in the room that aren't recorded in the
    //! timeline or state of the room. e.g. typing.
    Ephemeral ephemeral;
    //! The account_data events associated with this room.
    AccountData account_data;

    friend void from_json(const nlohmann::json &obj, JoinedRoom &room);
};

//! A room that the user has left or been banned from.
struct LeftRoom
{
    //! The state updates for the room up to the start of the timeline.
    State state;
    //! The timeline of messages and state changes in the room
    //! up to the point when the user left.
    Timeline timeline;

    friend void from_json(const nlohmann::json &obj, LeftRoom &room);
};

//! A room that the user has been invited to.
struct InvitedRoom
{
    //! The state of a room that the user has been invited to.
    //! These state events may only have the `sender`, `type`,
    //! `state_key` and `content` keys present.
    std::vector<events::collections::StrippedEvents> invite_state;
    //! Returns the name of the room.
    std::string name() const;
    //! Returns the URL for the avatar of the room.
    std::string avatar() const;

    friend void from_json(const nlohmann::json &obj, InvitedRoom &room);
};

//! A room that the user has knocked on.
struct KnockedRoom
{
    //! The state of a room that the user has knocked on.
    //! These state events may only have the `sender`, `type`,
    //! `state_key` and `content` keys present.
    std::vector<events::collections::StrippedEvents> knock_state;
    //! Returns the name of the room.
    std::string name() const;
    //! Returns the URL for the avatar of the room.
    std::string avatar() const;

    friend void from_json(const nlohmann::json &obj, KnockedRoom &room);
};

//! Room updates.
struct Rooms
{
    //! The rooms that the user has joined.
    std::map<std::string, JoinedRoom> join;
    //! The rooms that the user has left or been banned from.
    std::map<std::string, LeftRoom> leave;
    //! The rooms that the user has been invited to.
    std::map<std::string, InvitedRoom> invite;

    //! The rooms that the user has knocked on.
    std::map<std::string, KnockedRoom> knock;

    friend void from_json(const nlohmann::json &obj, Rooms &rooms);
};

//! Information on e2e device updates.
struct DeviceLists
{
    //! List of users who have updated their device identity keys
    //! since the previous sync response.
    std::vector<std::string> changed;
    //! List of users who may have left all the end-to-end encrypted
    //! rooms they previously shared with the user.
    std::vector<std::string> left;

    friend void from_json(const nlohmann::json &obj, DeviceLists &device_lists);
};

//! Information on to_device events in sync.
struct ToDevice
{
    //!  Information on the send-to-device messages for the client device.
    std::vector<events::collections::DeviceEvents> events;

    friend void from_json(const nlohmann::json &obj, ToDevice &to_device);
};

//! Response from the `GET /_matrix/client/r0/sync` endpoint.
struct Sync
{
    //! The batch token to supply in the since param of the next /sync request.
    std::string next_batch;
    //! Updates to rooms.
    Rooms rooms;
    //! Information on the send-to-device messages for the client device.
    ToDevice to_device;
    //! Information about presence of other users
    std::vector<mtx::events::Event<mtx::events::presence::Presence>> presence;
    //! Information on end-to-end device updates,
    DeviceLists device_lists;
    //! A mapping from algorithm to the number of one time keys
    //! the server has for the current device.
    std::map<std::string, uint16_t> device_one_time_keys_count;
    //! Required. The unused fallback key algorithms. Absence can be used to detect server support
    std::optional<std::vector<std::string>> device_unused_fallback_key_types;

    //! global account data
    AccountData account_data;

    friend void from_json(const nlohmann::json &obj, Sync &response);
};
}
}
