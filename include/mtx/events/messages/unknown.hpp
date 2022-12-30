#pragma once

/// @file
/// @brief An unknown message. According to the spec, this must have a text fallback.

#if __has_include(<nlohmann/json_fwd.hpp>)
#include <nlohmann/json_fwd.hpp>
#else
#include <nlohmann/json.hpp>
#endif

#include <string>

#include "mtx/events/common.hpp"

namespace mtx {
namespace events {
//! Non-state events sent in the timeline like messages.
namespace msg {

//! Content of `m.room.message` with an unrecognized msgtype.
struct Unknown
{
    //! The body of the message.
    std::string body;
    //! The message type.
    std::string msgtype;
    //! Relates to for rich replies
    mtx::common::Relations relations;

    friend void from_json(const nlohmann::json &obj, Unknown &content);
    friend void to_json(nlohmann::json &obj, const Unknown &content);
};

} // namespace msg
} // namespace events
} // namespace mtx
