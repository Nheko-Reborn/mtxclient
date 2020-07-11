#pragma once

#include <nlohmann/json.hpp>

#include <string>

namespace mtx {
namespace events {
namespace state {

//! Content for the `m.room.topic` state event.
//! A topic is a short message detailing what is currently being discussed in the room.
struct Topic
{
        //! The topic text.
        std::string topic;
};

void
from_json(const nlohmann::json &obj, Topic &event);

void
to_json(nlohmann::json &obj, const Topic &event);

} // namespace state
} // namespace events
} // namespace mtx
