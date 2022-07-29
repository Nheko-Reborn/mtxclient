#pragma once

/// @file
/// @brief A widget in account data or a room

#include <map>

#if __has_include(<nlohmann/json_fwd.hpp>)
#include <nlohmann/json_fwd.hpp>
#else
#include <nlohmann/json.hpp>
#endif

namespace mtx {
namespace events {
namespace state {
//! Content of the `m.widget` event.
///
/// https://github.com/matrix-org/matrix-doc/issues/1236
struct Widget
{
    //! Required. The type of widget. This is used to determine which widgets can be grouped
    //! together on the UI (e.g multiple grafana graphs together).
    std::string type;

    //! Required. The URL of the widget. All widget-enabled matrix clients need to transform this
    //! URL into a final URL which they will perform an HTTP GET to. All substituted values are
    //! url-encoded.
    std::string url;

    //! Optional. A human-readable string which can be displayed instead of, or in addition to, the
    //! embedded `<iframe>`.
    std::string name;

    //! Unique id
    std::string id;

    //! Optional (unless required by a widget type below). Arbitrary key/value pairs.
    std::map<std::string, std::string> data;

    //! This flag is used to denote whether the client should display a placeholder / loading
    //! spinner while waiting for the widget content to return a loaded event.
    bool waitForIframeLoad = true;

    //! The creatorUserID field is required. It is used to identify the user that added the widget
    //! instance (as opposed to the user that last modified the widget instance, as given by the
    //! 'sender' field on the m.widgets event).
    std::string creatorUserId;

    friend void from_json(const nlohmann::json &obj, Widget &create);
    friend void to_json(nlohmann::json &obj, const Widget &create);
};

} // namespace state
} // namespace events
} // namespace mtx
