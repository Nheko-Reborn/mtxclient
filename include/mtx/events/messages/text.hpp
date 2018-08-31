#pragma once

#include <json.hpp>
#include <string>

using json = nlohmann::json;

namespace mtx {
namespace events {
namespace msg {

struct Text
{
        // The body of the message.
        std::string body;
        // Must be 'm.text'.
        std::string msgtype;
};

void
from_json(const json &obj, Text &content);

void
to_json(json &obj, const Text &content);

} // namespace msg
} // namespace events
} // namespace mtx
