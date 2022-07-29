#pragma once

/// @file
/// @brief State event describing the visibility of the history.

#if __has_include(<nlohmann/json_fwd.hpp>)
#include <nlohmann/json_fwd.hpp>
#else
#include <nlohmann/json.hpp>
#endif

#include <string>

namespace mtx {
namespace events {
namespace state {
//! The different visibilities.
enum class Visibility
{
    //! All events while this is the `m.room.history_visibility`
    //! value may be shared by any participating homeserver with anyone,
    //! regardless of whether they have ever joined the room.
    WorldReadable,
    //! Previous events are always accessible to newly joined members.
    //! All events in the room are accessible, even those sent when
    //! the member was not a part of the room.
    Shared,
    //! Events are accessible to newly joined members from the point
    //! they were invited onwards. Events stop being accessible when
    //! the member's state changes to something other than invite or join.
    Invited,
    //! Events are accessible to newly joined members from the point
    //! they joined the room onwards. Events stop being accessible
    //! when the member's state changes to something other than join.
    Joined,
};

std::string
visibilityToString(const Visibility &rule);

Visibility
stringToVisibility(const std::string &rule);

//! Content of the `m.room.history_visibility` state event.
struct HistoryVisibility
{
    //! Who can see the room history.
    Visibility history_visibility;

    friend void from_json(const nlohmann::json &obj, HistoryVisibility &event);
    friend void to_json(nlohmann::json &obj, const HistoryVisibility &event);
};

} // namespace state
} // namespace events
} // namespace mtx
