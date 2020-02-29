#pragma once

#if __has_include(<nlohmann/json_fwd.hpp>)
#include <nlohmann/json_fwd.hpp>
#else
#include <nlohmann/json.hpp>
#endif

#include <string>
#include <vector>

#include "mtx/events/collections.hpp"

namespace mtx {
namespace responses {

struct EventId
{
        mtx::identifiers::Event event_id;
};

void
from_json(const nlohmann::json &obj, EventId &response);

struct GroupId
{
        std::string group_id;
};

void
from_json(const nlohmann::json &obj, GroupId &response);

struct FilterId
{
        std::string filter_id;
};

void
from_json(const nlohmann::json &obj, FilterId &response);

namespace utils {

using RoomAccountDataEvents = std::vector<mtx::events::collections::RoomAccountDataEvents>;
using TimelineEvents        = std::vector<mtx::events::collections::TimelineEvents>;
using StateEvents           = std::vector<mtx::events::collections::StateEvents>;
using StrippedEvents        = std::vector<mtx::events::collections::StrippedEvents>;

namespace states = mtx::events::state;
namespace msgs   = mtx::events::msg;

void
log_error(nlohmann::json::exception &err, const nlohmann::json &event);

void
log_error(std::string err, const nlohmann::json &event);

void
parse_room_account_data_events(const nlohmann::json &events, RoomAccountDataEvents &container);

void
compose_timeline_events(nlohmann::json &events, const TimelineEvents &container);

void
parse_timeline_events(const nlohmann::json &events, TimelineEvents &container);

void
parse_state_events(const nlohmann::json &events, StateEvents &container);

void
parse_stripped_events(const nlohmann::json &events, StrippedEvents &container);
}
}
}
