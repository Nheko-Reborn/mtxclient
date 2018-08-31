#pragma once

#include <json.hpp>
#include <string>

using json = nlohmann::json;

namespace mtx {
namespace events {
namespace msg {

struct Notice
{
        // The notice text to send.
        std::string body;
        // Must be 'm.notice'.
        std::string msgtype;
};

void
from_json(const json &obj, Notice &content);

void
to_json(json &obj, const Notice &content);

} // namespace msg
} // namespace events
} // namespace mtx
