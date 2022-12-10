#pragma once

/// @file
/// @brief A message that appears to be a text message but doesn't have msgtype m.text.

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

//! Content of `m.room.message` without a msgtype `m.text`.
struct DuckTypesToText
{
    //! The body of the message.
    std::string body;
    //! The type of the message.
    std::string msgtype;
    //! We only handle org.matrix.custom.html.
    std::string format;
    //! HTML formatted message.
    std::string formatted_body;
    //! Relates to for rich replies
    mtx::common::Relations relations;

    friend void from_json(const nlohmann::json &obj, DuckTypesToText &content);
    friend void to_json(nlohmann::json &obj, const DuckTypesToText &content);
};

} // namespace msg
} // namespace events
} // namespace mtx
