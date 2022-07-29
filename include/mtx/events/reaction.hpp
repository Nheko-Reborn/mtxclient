#pragma once

/// @file
/// @brief A reaction event used to attach small annotations to events.

#if __has_include(<nlohmann/json_fwd.hpp>)
#include <nlohmann/json_fwd.hpp>
#else
#include <nlohmann/json.hpp>
#endif

#include <string>

#include "mtx/events/common.hpp"

namespace mtx {
namespace events {
namespace msg {

//! Content for the `m.reaction` event.
struct Reaction
{
    //! Should be an annotation relation
    common::Relations relations;

    friend void from_json(const nlohmann::json &obj, Reaction &event);
    friend void to_json(nlohmann::json &obj, const Reaction &event);
};

} // namespace msg
} // namespace events
} // namespace mtx
