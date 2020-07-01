#pragma once

#include <nlohmann/json.hpp>

#include <string>

#include "mtx/events/common.hpp"

namespace mtx {
namespace events {
namespace msg {

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
        mtx::common::ReplyRelatesTo relates_to;
};

void
from_json(const nlohmann::json &obj, Text &content);

void
to_json(nlohmann::json &obj, const Text &content);

} // namespace msg
} // namespace events
} // namespace mtx
