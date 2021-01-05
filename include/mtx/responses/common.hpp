#pragma once

/// @file
/// @brief Responses used by multiple endpoints.

#if __has_include(<nlohmann/json_fwd.hpp>)
#include <nlohmann/json_fwd.hpp>
#else
#include <nlohmann/json.hpp>
#endif

#include <string>
#include <vector>

#include "mtx/events/collections.hpp"

namespace mtx {
//! Namespace for the different types of responses.
namespace responses {
//! An event id returned by the API.
struct EventId
{
        //! The event id.
        mtx::identifiers::Event event_id;
};

void
from_json(const nlohmann::json &obj, EventId &response);

//! A group id returned by the API.
struct GroupId
{
        //! The group id.
        std::string group_id;
};

void
from_json(const nlohmann::json &obj, GroupId &response);

//! A room id returned by the API.
struct RoomId
{
        //! The room id.
        std::string room_id;
};

void
from_json(const nlohmann::json &obj, RoomId &response);

//! A filter id returned by the API.
struct FilterId
{
        //! The filter id.
        std::string filter_id;
};

void
from_json(const nlohmann::json &obj, FilterId &response);

//! Different helper for parsing responses.
namespace utils {
//! Multiple account_data events.
using RoomAccountDataEvents = std::vector<mtx::events::collections::RoomAccountDataEvents>;
//! Multiple TimelineEvents.
using TimelineEvents = std::vector<mtx::events::collections::TimelineEvents>;
//! Multiple StateEvents.
using StateEvents = std::vector<mtx::events::collections::StateEvents>;
//! Multiple StrippedEvents.
using StrippedEvents = std::vector<mtx::events::collections::StrippedEvents>;
//! Multiple DeviceEvents.
using DeviceEvents = std::vector<mtx::events::collections::DeviceEvents>;
//! Multiple EphemeralEvents.
using EphemeralEvents = std::vector<mtx::events::collections::EphemeralEvents>;

namespace states = mtx::events::state;
namespace msgs   = mtx::events::msg;

void
log_error(std::exception &err, const nlohmann::json &event);

void
log_error(std::string err, const nlohmann::json &event);

//! Parse multiple account_data events.
void
parse_room_account_data_events(const nlohmann::json &events, RoomAccountDataEvents &container);

void
compose_timeline_events(nlohmann::json &events, const TimelineEvents &container);

//! Parse multiple timeline events.
void
parse_timeline_events(const nlohmann::json &events, TimelineEvents &container);

//! Parse multiple state events.
void
parse_state_events(const nlohmann::json &events, StateEvents &container);

//! Parse multiple stripped events.
void
parse_stripped_events(const nlohmann::json &events, StrippedEvents &container);

//! Parse multiple device events.
void
parse_device_events(const nlohmann::json &events, DeviceEvents &container);

//! Parse multiple ephemeral events.
void
parse_ephemeral_events(const nlohmann::json &events, EphemeralEvents &container);
}
}
}
