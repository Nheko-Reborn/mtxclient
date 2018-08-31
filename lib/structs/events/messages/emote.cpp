#include <json.hpp>
#include <string>

#include "mtx/events/messages/emote.hpp"

using json = nlohmann::json;

namespace mtx {
namespace events {
namespace msg {

void
from_json(const json &obj, Emote &content)
{
        content.body    = obj.at("body").get<std::string>();
        content.msgtype = obj.at("msgtype").get<std::string>();
}

void
to_json(json &obj, const Emote &content)
{
        obj["msgtype"] = "m.emote";
        obj["body"]    = content.body;
}

} // namespace msg
} // namespace events
} // namespace mtx
