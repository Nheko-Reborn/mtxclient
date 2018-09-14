#include <nlohmann/json.hpp>
#include <string>

#include "mtx/events/messages/image.hpp"

using json = nlohmann::json;

namespace common = mtx::common;

namespace mtx {
namespace events {
namespace msg {

void
from_json(const json &obj, Image &content)
{
        content.body    = obj.at("body").get<std::string>();
        content.msgtype = obj.at("msgtype").get<std::string>();

        if (obj.find("url") != obj.end())
                content.url = obj.at("url").get<std::string>();

        if (obj.find("info") != obj.end())
                content.info = obj.at("info").get<common::ImageInfo>();
}

void
to_json(json &obj, const Image &content)
{
        obj["msgtype"] = "m.image";
        obj["body"]    = content.body;
        obj["url"]     = content.url;
        obj["info"]    = content.info;
}

void
from_json(const json &obj, StickerImage &content)
{
        content.body = obj.at("body").get<std::string>();

        if (obj.find("url") != obj.end())
                content.url = obj.at("url").get<std::string>();

        if (obj.find("info") != obj.end())
                content.info = obj.at("info").get<common::ImageInfo>();
}

void
to_json(json &obj, const StickerImage &content)
{
        obj["body"] = content.body;
        obj["url"]  = content.url;
        obj["info"] = content.info;
}

} // namespace msg
} // namespace events
} // namespace mtx
