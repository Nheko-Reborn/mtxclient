#pragma once

/// @file
/// @brief The state event describing the topic in a room.

#if __has_include(<nlohmann/json_fwd.hpp>)
#include <nlohmann/json_fwd.hpp>
#else
#include <nlohmann/json.hpp>
#endif

#include <string>

namespace mtx {
namespace events {
namespace state {

/// @brief Content for the `m.room.topic` state event.
//
/// A topic is a short message detailing what is currently being discussed in the room.
struct Topic
{
    //! The topic text.
    std::string topic;

    friend void from_json(const nlohmann::json &obj, Topic &event);
    friend void to_json(nlohmann::json &obj, const Topic &event);
};

} // namespace state
} // namespace events
} // namespace mtx
