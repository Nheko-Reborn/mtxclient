#include <nlohmann/json.hpp>
#include <string>

#include "mtx/events/common.hpp"
#include "mtx/events/messages/text.hpp"

using json = nlohmann::json;

namespace mtx {
namespace events {
namespace msg {

void
from_json(const json &obj, Text &content)
{
        content.body    = obj.at("body").get<std::string>();
        content.msgtype = obj.at("msgtype").get<std::string>();

        if (obj.count("format") != 0)
                content.format = obj.at("format").get<std::string>();

        if (obj.count("formatted_body") != 0)
                content.formatted_body = obj.at("formatted_body").get<std::string>();

        if (obj.count("m.relates_to") != 0)
                content.relates_to = obj.at("m.relates_to").get<common::RelatesTo>();
}

void
to_json(json &obj, const Text &content)
{
        obj["msgtype"] = "m.text";
        obj["body"]    = content.body;

        if (!content.formatted_body.empty()) {
                obj["format"]         = mtx::common::FORMAT_MSG_TYPE;
                obj["formatted_body"] = content.formatted_body;
        }

        if (!content.relates_to.in_reply_to.event_id.empty())
                obj["m.relates_to"] = content.relates_to;
}

} // namespace msg
} // namespace events
} // namespace mtx
