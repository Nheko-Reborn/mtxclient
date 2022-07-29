#pragma once

/// @file
/// @brief Notification related endpoints.

#if __has_include(<nlohmann/json_fwd.hpp>)
#include <nlohmann/json_fwd.hpp>
#else
#include <nlohmann/json.hpp>
#endif

#include "mtx/events/collections.hpp"
#include "mtx/pushrules.hpp"

namespace mtx {
namespace responses {

//! Description for a notification.
struct Notification
{
    //! The action to perform when the conditions for this rule are met.
    std::vector<mtx::pushrules::actions::Action> actions;
    //! The Event object for the event that triggered the notification.
    mtx::events::collections::TimelineEvents event;
    //! Indicates whether the user has sent a read receipt indicating
    //! that they have read this message.
    bool read = false;
    //! The profile tag of the rule that matched this event.
    std::string profile_tag;
    //! The ID of the room in which the event was posted.
    std::string room_id;
    //! The unix timestamp at which the event notification was sent, in milliseconds.
    uint64_t ts;

    friend void from_json(const nlohmann::json &obj, Notification &res);
    friend void to_json(nlohmann::json &obj, const Notification &res);
};

//! Response from the `GET /_matrix/client/r0/notifications` endpoint.
//
//! The endpoint is used to paginate through the list of events
//! that the user has been, or would have been notified about.
struct Notifications
{
    //! The token to supply in the from param of the next /notifications
    //! request in order to request more events. If this is absent,
    //! there are no more results.
    //! TODO: https://github.com/matrix-org/synapse/pull/3190
    // std::string next_token;
    //! The list of events that triggered notifications.
    std::vector<Notification> notifications;

    friend void from_json(const nlohmann::json &obj, Notifications &res);
    friend void to_json(nlohmann::json &obj, const Notifications &res);
};
}
}
