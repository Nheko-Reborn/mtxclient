#pragma once

#include <nlohmann/json.hpp>
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
        //! We only handle org.matrix.custom.html.
        std::string format;
        //! HTML formatted message.
        std::string formatted_body;
};

void
from_json(const json &obj, Emote &content);

void
to_json(json &obj, const Emote &content);

} // namespace msg
} // namespace events
} // namespace mtx
