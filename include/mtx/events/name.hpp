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
//! Events, that can be used as a state event.
namespace state {

//! Content of the `m.room.name` event.
struct Name
{
    //! The name of the room.
    std::string name;

    friend void from_json(const nlohmann::json &obj, Name &event);
    friend void to_json(nlohmann::json &obj, const Name &event);
};

} // namespace state
} // namespace events
} // namespace mtx
