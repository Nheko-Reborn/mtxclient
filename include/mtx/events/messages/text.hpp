#pragma once

/// @file
/// @brief A text message.

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

//! Content of `m.room.message` with msgtype `m.text`.
struct Text
{
    //! The body of the message.
    std::string body;
    //! Must be 'm.text'.
    std::string msgtype;
    //! We only handle org.matrix.custom.html.
    std::string format;
    //! HTML formatted message.
    std::string formatted_body;
    //! Relates to for rich replies
    mtx::common::Relations relations;

    friend void from_json(const nlohmann::json &obj, Text &content);
    friend void to_json(nlohmann::json &obj, const Text &content);
};

} // namespace msg
} // namespace events
} // namespace mtx
