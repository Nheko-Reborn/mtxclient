#include <nlohmann/json.hpp>
#include <string>

#include "mtx/events/common.hpp"
#include "mtx/events/messages/notice.hpp"

using json = nlohmann::json;

namespace mtx {
namespace events {
namespace msg {

void
from_json(const json &obj, Notice &content)
{
        content.body    = obj.at("body").get<std::string>();
        content.msgtype = obj.at("msgtype").get<std::string>();

        if (obj.count("format") != 0)
                content.format = obj.at("format").get<std::string>();

        if (obj.count("formatted_body") != 0)
                content.formatted_body = obj.at("formatted_body").get<std::string>();

        if (obj.count("m.relates_to") != 0)
                content.relates_to = obj.at("m.relates_to").get<common::ReplyRelatesTo>();
}

void
to_json(json &obj, const Notice &content)
{
        obj["msgtype"] = "m.notice";
        obj["body"]    = content.body;

        if (!content.formatted_body.empty()) {
                obj["format"]         = mtx::common::FORMAT_MSG_TYPE;
                obj["formatted_body"] = content.formatted_body;
        }

        obj["m.relates_to"] = content.relates_to;
}

} // namespace msg
} // namespace events
} // namespace mtx
