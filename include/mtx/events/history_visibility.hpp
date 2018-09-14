#pragma once

#include <nlohmann/json.hpp>
#include <string>

using json = nlohmann::json;

namespace mtx {
namespace events {
namespace state {

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
};

void
from_json(const json &obj, HistoryVisibility &event);

void
to_json(json &obj, const HistoryVisibility &event);

} // namespace state
} // namespace events
} // namespace mtx
