#pragma once

/// @file
/// @brief The room name state event.

#if __has_include(<nlohmann/json_fwd.hpp>)
#include <nlohmann/json_fwd.hpp>
#else
#include <nlohmann/json.hpp>
#endif

#include <string>

namespace mtx {
//! Namespace for all events.
namespace events {
//! Placeholder event for if an event is unsupported
struct Unknown
{
    //! The original content
    std::string content;
    //! The original type
    std::string type;

    friend void from_json(const nlohmann::json &obj, Unknown &event);
    friend void to_json(nlohmann::json &obj, const Unknown &event);
};

} // namespace events
} // namespace mtx
