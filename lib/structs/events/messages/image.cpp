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
    content.body    = obj.value("body", "");
    content.msgtype = obj.at("msgtype").get<std::string>();

    content.url = obj.value("url", "");

    if (obj.find("info") != obj.end())
        content.info = obj.at("info").get<common::ImageInfo>();

    if (obj.find("file") != obj.end())
        content.file = obj.at("file").get<crypto::EncryptedFile>();

    content.relations = common::parse_relations(obj);
}

void
to_json(json &obj, const Image &content)
{
    obj["msgtype"] = "m.image";
    obj["body"]    = content.body;
    obj["info"]    = content.info;

    if (content.file)
        obj["file"] = content.file.value();
    else
        obj["url"] = content.url;

    common::apply_relations(obj, content.relations);
}

void
from_json(const json &obj, StickerImage &content)
{
    content.body = obj.value("body", "");

    content.url = obj.value("url", "");

    if (obj.find("info") != obj.end())
        content.info = obj.at("info").get<common::ImageInfo>();

    if (obj.find("file") != obj.end())
        content.file = obj.at("file").get<crypto::EncryptedFile>();

    content.relations = common::parse_relations(obj);
}

void
to_json(json &obj, const StickerImage &content)
{
    obj["body"] = content.body;
    obj["info"] = content.info;

    if (content.file)
        obj["file"] = content.file.value();
    else
        obj["url"] = content.url;

    common::apply_relations(obj, content.relations);
}

} // namespace msg
} // namespace events
} // namespace mtx
