#pragma once

/// @file
/// @brief Events pinned in a room.

#if __has_include(<nlohmann/json_fwd.hpp>)
#include <nlohmann/json_fwd.hpp>
#else
#include <nlohmann/json.hpp>
#endif

#include <string>

namespace mtx {
namespace events {
namespace state {

//! `m.room.pinned_events` state event.
struct PinnedEvents
{
    //! The ids of the pinned events.
    std::vector<std::string> pinned;

    friend void from_json(const nlohmann::json &obj, PinnedEvents &event);
    friend void to_json(nlohmann::json &obj, const PinnedEvents &event);
};

} // namespace state
} // namespace events
} // namespace mtx
