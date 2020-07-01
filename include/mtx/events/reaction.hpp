#pragma once

#include <nlohmann/json.hpp>

#include <string>

#include "mtx/events/common.hpp"

namespace mtx {
namespace events {
namespace msg {

//! Content for the `m.reaction` event.
struct Reaction
{
        //! The event being reacted to
        mtx::common::ReactionRelatesTo relates_to;
};

void
from_json(const nlohmann::json &obj, Reaction &event);

void
to_json(nlohmann::json &obj, const Reaction &event);

} // namespace msg
} // namespace events
} // namespace mtx
