#pragma once

/// @file
/// @brief A bot generated message and other notices.

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

//! Content of `m.room.message` with msgtype `m.notice`.
struct Notice
{
    //! The notice text to send.
    std::string body;
    //! Must be 'm.notice'.
    std::string msgtype;
    //! We only handle org.matrix.custom.html.
    std::string format;
    //! HTML formatted message.
    std::string formatted_body;
    //! Relates to for rich replies
    mtx::common::Relations relations;

    friend void from_json(const nlohmann::json &obj, Notice &content);
    friend void to_json(nlohmann::json &obj, const Notice &content);
};

} // namespace msg
} // namespace events
} // namespace mtx
