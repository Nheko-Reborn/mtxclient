#pragma once

#include <map>
#include <string>
#include <vector>

#include "mtx/events.hpp"
#include "mtx/events/collections.hpp"
#include "mtx/identifiers.hpp"
#include <nlohmann/json.hpp>

using json = nlohmann::json;

namespace mtx {
namespace responses {

//! Room specific Account Data events.
struct RoomAccountData
{
        //! List of events.
        std::vector<events::collections::RoomAccountDataEvents> events;
};

void
from_json(const json &obj, RoomAccountData &account_data);

//! State events.
struct State
{
        //! List of events.
        std::vector<events::collections::StateEvents> events;
};

void
from_json(const json &obj, State &state);

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
};

void
from_json(const json &obj, Timeline &timeline);

//! Counts of unread notifications for this room
struct UnreadNotifications
{
        //! The number of unread notifications for this room with the
        //! highlight flag set.
        uint16_t highlight_count = 0;
        //! The total number of unread notifications for this room.
        uint16_t notification_count = 0;
};

void
from_json(const json &obj, UnreadNotifications &notifications);

//! The ephemeral events in the room that aren't recorded in
//! the timeline or state of the room. e.g. typing.
struct Ephemeral
{
        //! A list of users that are currently typing.
        std::vector<std::string> typing;
        //! Map of events and the users that have read them.
        std::map<std::string, std::map<std::string, uint64_t>> receipts;
};

void
from_json(const json &obj, Ephemeral &ephemeral);

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
        RoomAccountData account_data;
};

void
from_json(const json &obj, JoinedRoom &room);

//! A room that the user has left or been banned from.
struct LeftRoom
{
        //! The state updates for the room up to the start of the timeline.
        State state;
        //! The timeline of messages and state changes in the room
        //! up to the point when the user left.
        Timeline timeline;
};

void
from_json(const json &obj, LeftRoom &room);

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
};

void
from_json(const json &obj, InvitedRoom &room);

//! Room updates.
struct Rooms
{
        //! The rooms that the user has joined.
        std::map<std::string, JoinedRoom> join;
        //! The rooms that the user has left or been banned from.
        std::map<std::string, LeftRoom> leave;
        //! The rooms that the user has been invited to.
        std::map<std::string, InvitedRoom> invite;
};

void
from_json(const json &obj, Rooms &rooms);

//! Information on e2e device updates.
struct DeviceLists
{
        //! List of users who have updated their device identity keys
        //! since the previous sync response.
        std::vector<std::string> changed;
        //! List of users who may have left all the end-to-end encrypted
        //! rooms they previously shared with the user.
        std::vector<std::string> left;
};

void
from_json(const json &obj, DeviceLists &device_lists);

//! Response from the `GET /_matrix/client/r0/sync` endpoint.
struct Sync
{
        //! The batch token to supply in the since param of the next /sync request.
        std::string next_batch;
        //! Updates to rooms.
        Rooms rooms;
        //! Information on the send-to-device messages for the client device.
        std::vector<nlohmann::json> to_device;
        /* Presence presence; */
        /* Groups groups; */
        //! Information on end-to-end device updates,
        DeviceLists device_lists;
        //! A mapping from algorithm to the number of one time keys
        //! the server has for the current device.
        std::map<std::string, uint16_t> device_one_time_keys_count;
        /* AccountData account_data; */
};

void
from_json(const json &obj, Sync &response);
}
}
