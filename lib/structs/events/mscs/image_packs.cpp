#include "mtx/events/mscs/image_packs.hpp"

#include <nlohmann/json.hpp>

namespace mtx {
namespace events {
namespace msc2545 {
void
from_json(const nlohmann::json &obj, PackImage &content)
{
        content.url  = obj.at("url");
        content.body = obj.value("body", "");
        if (obj.contains("info"))
                content.info = obj.at("info").get<mtx::common::ImageInfo>();

        if (obj.contains("usage")) {
                for (const auto &e : obj.at("usage")) {
                        if (e == "sticker")
                                content.usage.set(PackUsage::Sticker);
                        else if (e == "emoticon")
                                content.usage.set(PackUsage::Emoji);
                }
        }
}

void
from_json(const nlohmann::json &obj, ImagePack::PackDescription &content)
{
        content.avatar_url   = obj.value("avatar_url", "");
        content.display_name = obj.value("display_name", "");
        content.attribution  = obj.value("attribution", "");

        if (obj.contains("usage")) {
                for (const auto &e : obj.at("usage")) {
                        if (e == "sticker")
                                content.usage.set(PackUsage::Sticker);
                        else if (e == "emoticon")
                                content.usage.set(PackUsage::Emoji);
                }
        }
}

void
from_json(const nlohmann::json &obj, ImagePack &content)
{
        if (obj.contains("pack")) {
                content.pack = ImagePack::PackDescription{};
                from_json(obj.at("pack"), *content.pack);
        }
        content.images = obj.at("images").get<std::map<std::string, PackImage>>();
}

void
to_json(nlohmann::json &obj, const PackImage &content)
{
        obj["url"] = content.url;

        if (!content.body.empty())
                obj["body"] = content.body;
        if (content.info)
                obj["info"] = content.info.value();

        if (content.usage.test(PackUsage::Sticker))
                obj["usage"].push_back("sticker");
        if (content.usage.test(PackUsage::Emoji))
                obj["usage"].push_back("emoticon");
}
void
to_json(nlohmann::json &obj, const ImagePack::PackDescription &content)
{
        if (!content.avatar_url.empty())
                obj["avatar_url"] = content.avatar_url;
        if (!content.display_name.empty())
                obj["display_name"] = content.display_name;
        if (!content.attribution.empty())
                obj["attribution"] = content.attribution;

        if (content.usage.test(PackUsage::Sticker))
                obj["usage"].push_back("sticker");
        if (content.usage.test(PackUsage::Emoji))
                obj["usage"].push_back("emoticon");
}
void
to_json(nlohmann::json &obj, const ImagePack &content)
{
        if (content.pack)
                to_json(obj["pack"], *content.pack);

        obj["images"] = content.images;
}

void
from_json(const nlohmann::json &obj, ImagePackRooms &content)
{
        for (const auto &[roomid, packs] : obj["rooms"].items()) {
                for (const auto &[packid, body] : packs.items()) {
                        content.rooms[roomid][packid] = body.dump();
                }
        }
}
void
to_json(nlohmann::json &obj, const ImagePackRooms &content)
{
        for (const auto &[roomid, packs] : content.rooms) {
                for (const auto &[packid, body] : packs) {
                        if (body.empty())
                                obj["rooms"][roomid][packid] = json::object();
                        else
                                obj["rooms"][roomid][packid] = json::parse(body);
                }
        }
}
}
}
}
