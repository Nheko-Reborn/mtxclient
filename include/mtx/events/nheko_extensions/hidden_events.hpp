#pragma once

/// @file
/// @brief A nheko specific event in account data used to hide events across different clients.

#include <vector>

#if __has_include(<nlohmann/json_fwd.hpp>)
#include <nlohmann/json_fwd.hpp>
#else
#include <nlohmann/json.hpp>
#endif

#include "mtx/events.hpp"

namespace mtx {
namespace events {
namespace account_data {
//! Custom nheko specific event types
namespace nheko_extensions {
//! Custom event to hide certain event types in nheko
struct HiddenEvents
{
    //! The hidden event types
    std::optional<std::vector<EventType>> hidden_event_types;

    friend void from_json(const nlohmann::json &obj, HiddenEvents &content);
    friend void to_json(nlohmann::json &obj, const HiddenEvents &content);
};
}
} // namespace account_data
} // namespace events
} // namespace mtx
