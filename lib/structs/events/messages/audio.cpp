#include <nlohmann/json.hpp>
#include <string>

#include "mtx/events/messages/audio.hpp"

using json = nlohmann::json;

namespace common = mtx::common;

namespace mtx {
namespace events {
namespace msg {

void
from_json(const json &obj, Audio &content)
{
        content.body    = obj.at("body").get<std::string>();
        content.msgtype = obj.at("msgtype").get<std::string>();

        if (obj.find("url") != obj.end())
                content.url = obj.at("url").get<std::string>();

        if (obj.find("info") != obj.end())
                content.info = obj.at("info").get<common::AudioInfo>();
}

void
to_json(json &obj, const Audio &content)
{
        obj["msgtype"] = "m.audio";
        obj["body"]    = content.body;
        obj["url"]     = content.url;
        obj["info"]    = content.info;
}

} // namespace msg
} // namespace events
} // namespace mtx
