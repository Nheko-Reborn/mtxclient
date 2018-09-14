#pragma once

#include <nlohmann/json.hpp>

#include <string>
#include <vector>

#include "mtx/events/collections.hpp"

using json = nlohmann::json;

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

void
log_error(json::exception &err, const json &event);

void
log_error(std::string err, const json &event);

void
parse_room_account_data_events(const json &events, RoomAccountDataEvents &container);

void
parse_timeline_events(const json &events, TimelineEvents &container);

void
parse_state_events(const json &events, StateEvents &container);

void
parse_stripped_events(const json &events, StrippedEvents &container);
}
}
}
