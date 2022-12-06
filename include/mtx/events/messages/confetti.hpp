#pragma once

/// @file
/// @brief A confetti message.

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

//! Content of `m.room.message` with msgtype `nic.custom.confetti`.
struct Confetti
{
    //! The body of the message.
    std::string body;
    //! Must be 'nic.custom.confetti'.
    std::string msgtype;
    //! We only handle org.matrix.custom.html.
    std::string format;
    //! HTML formatted message.
    std::string formatted_body;
    //! Relates to for rich replies
    mtx::common::Relations relations;

    friend void from_json(const nlohmann::json &obj, Confetti &content);
    friend void to_json(nlohmann::json &obj, const Confetti &content);
};

} // namespace msg
} // namespace events
} // namespace mtx
