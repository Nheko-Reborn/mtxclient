#pragma once

#include <nlohmann/json.hpp>

#include <string>

#include <mtx/events/common.hpp>

namespace mtx {
namespace events {
namespace msg {

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
        mtx::common::ReplyRelatesTo relates_to;
};

void
from_json(const nlohmann::json &obj, Emote &content);

void
to_json(nlohmann::json &obj, const Emote &content);

} // namespace msg
} // namespace events
} // namespace mtx
