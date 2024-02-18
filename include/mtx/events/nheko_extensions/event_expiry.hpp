#pragma once

/// @file
/// @brief A nheko specific event in account data used to delete events meeting certain criteria

#if __has_include(<nlohmann/json_fwd.hpp>)
#include <nlohmann/json_fwd.hpp>
#else
#include <nlohmann/json.hpp>
#endif

namespace mtx {
namespace events {
namespace account_data {
//! Custom nheko specific event types
namespace nheko_extensions {
//! Custom event to delete certain events automatically (e.g. after a certain time). Can be
//! configured globally or per room.
struct EventExpiry
{
    //! If state events should be protected from deletion. Defaults to false, however the latest
    //! state event of a specific (event_type,state_key) is always protected.
    bool exclude_state_events = false;

    //! When to delete old events. Set to 0 or remove to not delete events based on time.
    //! Uses the origin_server_ts so keep your clocks in sync. Too small of an expiration timeout
    //! might be annoying.
    std::uint64_t expire_after_ms = 0;

    //! Protect recent N messages from deletion, even if other criteria match.
    //! By default no messages are protected (but you need to configure a criteria to match anything
    //! anyway). This can be used to be less disruptive in recent conversations.
    std::uint64_t protect_latest = 0;

    //! Deletes anything but the latest N messages. Set to 0 to disable (because keeping no messages
    //! makes no sense, since then you would just send nothing). If this is lower than
    //! `protect_latest`, it will effectively default to that (but will be disabled when set to 0).
    //! Clients might run deletions only on a certain interval.
    std::uint64_t keep_only_latest = 0;

    friend void from_json(const nlohmann::json &obj, EventExpiry &content);
    friend void to_json(nlohmann::json &obj, const EventExpiry &content);
};
}
} // namespace account_data
} // namespace events
} // namespace mtx
