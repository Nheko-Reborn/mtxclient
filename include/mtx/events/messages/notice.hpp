#pragma once

#include <nlohmann/json.hpp>

#include <string>

#include "mtx/events/common.hpp"

namespace mtx {
namespace events {
namespace msg {

struct Notice
{
        // The notice text to send.
        std::string body;
        // Must be 'm.notice'.
        std::string msgtype;
        //! We only handle org.matrix.custom.html.
        std::string format;
        //! HTML formatted message.
        std::string formatted_body;
        // Relates to for rich replies
        mtx::common::ReplyRelatesTo relates_to;
};

void
from_json(const nlohmann::json &obj, Notice &content);

void
to_json(nlohmann::json &obj, const Notice &content);

} // namespace msg
} // namespace events
} // namespace mtx
