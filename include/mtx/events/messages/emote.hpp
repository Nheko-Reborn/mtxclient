#pragma once

/// @file
/// @brief Events describing an emotion or action.

#if __has_include(<nlohmann/json_fwd.hpp>)
#include <nlohmann/json_fwd.hpp>
#else
#include <nlohmann/json.hpp>
#endif

#include <string>

#include <mtx/events/common.hpp>

namespace mtx {
namespace events {
namespace msg {

//! Content of `m.room.message` with msgtype `m.emote`.
struct Emote
{
    // The emote action to perform.
    std::string body;
    // Must be 'm.emote'.
    std::string msgtype;
    //! We only handle org.matrix.custom.html.
    std::string format;
    //! HTML formatted message.
    std::string formatted_body;
    //! Relates to for rich replies
    mtx::common::Relations relations;

    friend void from_json(const nlohmann::json &obj, Emote &content);
    friend void to_json(nlohmann::json &obj, const Emote &content);
};

} // namespace msg
} // namespace events
} // namespace mtx
