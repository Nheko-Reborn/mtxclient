#pragma once

#include <json.hpp>
#include <string>

using json = nlohmann::json;

namespace mtx {
namespace events {
namespace msg {

struct Emote
{
        // The emote action to perform.
        std::string body;
        // Must be 'm.emote'.
        std::string msgtype;
};

void
from_json(const json &obj, Emote &content);

void
to_json(json &obj, const Emote &content);

} // namespace msg
} // namespace events
} // namespace mtx
