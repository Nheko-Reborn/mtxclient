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

    friend void from_json(const nlohmann::json &obj, EventId &response);
};

//! A room id returned by the API.
struct RoomId
{
    //! The room id.
    std::string room_id;

    friend void from_json(const nlohmann::json &obj, RoomId &response);
};

//! A filter id returned by the API.
struct FilterId
{
    //! The filter id.
    std::string filter_id;

    friend void from_json(const nlohmann::json &obj, FilterId &response);
};

//! A new room version as returned by the room_keys/version API
struct Version
{
    //! Required: The backup version. This is an opaque string.
    std::string version;

    friend void from_json(const nlohmann::json &obj, Version &response);
};

//! Some endpoints return this to indicate success in addition to the http code.
struct Success
{
    //! Required. Whether the validation was successful or not.
    bool success;

    friend void from_json(const nlohmann::json &obj, Success &response);
};

//! Some endpoints return this to indicate availability in addition to the http code (i.e. a
//! username).
struct Available
{
    //! Required. A flag to indicate that the resource is available.
    bool available;

    friend void from_json(const nlohmann::json &obj, Available &response);
};

//! Responses to the `/requestToken` endpoints
struct RequestToken
{
    //! Required. The session ID. Session IDs are opaque strings that must consist entirely of the
    //! characters [0-9a-zA-Z.=_-]. Their length must not exceed 255 characters and they must not be
    //! empty.
    std::string sid;
    //! An optional field containing a URL where the client must submit the validation token to,
    //! with identical parameters to the Identity Service API's POST /validate/email/submitToken
    //! endpoint (without the requirement for an access token). The homeserver must send this token
    //! to the user (if applicable), who should then be prompted to provide it to the client.
    std::string submit_url;

    friend void from_json(const nlohmann::json &obj, RequestToken &response);
};

//! A simple list of aliases
struct Aliases
{
    //! The aliases
    std::vector<std::string> aliases;

    friend void from_json(const nlohmann::json &obj, Aliases &response);
};

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
log_error(const std::string &err, const nlohmann::json &event);

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

//! An array of state events
struct StateEvents
{
    utils::StateEvents events;

    friend void from_json(const nlohmann::json &arr, StateEvents &response);
};
}
}
