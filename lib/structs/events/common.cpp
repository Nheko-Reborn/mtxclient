#include <nlohmann/json.hpp>

#include "mtx/events/common.hpp"

using json = nlohmann::json;

namespace mtx {
namespace common {

void
from_json(const json &obj, ThumbnailInfo &info)
{
        if (obj.find("h") != obj.end())
                info.h = obj.at("h").get<uint64_t>();

        if (obj.find("w") != obj.end())
                info.w = obj.at("w").get<uint64_t>();

        if (obj.find("size") != obj.end())
                info.size = obj.at("size").get<uint64_t>();

        if (obj.find("mimetype") != obj.end())
                info.mimetype = obj.at("mimetype").get<std::string>();
}

void
to_json(json &obj, const ThumbnailInfo &info)
{
        obj["h"]        = info.h;
        obj["w"]        = info.w;
        obj["size"]     = info.size;
        obj["mimetype"] = info.mimetype;
}

void
from_json(const json &obj, ImageInfo &info)
{
        if (obj.find("h") != obj.end())
                info.h = obj.at("h").get<uint64_t>();

        if (obj.find("w") != obj.end())
                info.w = obj.at("w").get<uint64_t>();

        if (obj.find("size") != obj.end())
                info.size = obj.at("size").get<uint64_t>();

        if (obj.find("mimetype") != obj.end())
                info.mimetype = obj.at("mimetype").get<std::string>();

        if (obj.find("thumbnail_url") != obj.end())
                info.thumbnail_url = obj.at("thumbnail_url").get<std::string>();

        if (obj.find("thumbnail_info") != obj.end())
                info.thumbnail_info = obj.at("thumbnail_info").get<ThumbnailInfo>();
}

void
to_json(json &obj, const ImageInfo &info)
{
        obj["h"]              = info.h;
        obj["w"]              = info.w;
        obj["size"]           = info.size;
        obj["mimetype"]       = info.mimetype;
        obj["thumbnail_url"]  = info.thumbnail_url;
        obj["thumbnail_info"] = info.thumbnail_info;
}

void
from_json(const json &obj, FileInfo &info)
{
        if (obj.find("size") != obj.end())
                info.size = obj.at("size").get<uint64_t>();

        if (obj.find("mimetype") != obj.end())
                info.mimetype = obj.at("mimetype").get<std::string>();

        if (obj.find("thumbnail_url") != obj.end())
                info.thumbnail_url = obj.at("thumbnail_url").get<std::string>();

        if (obj.find("thumbnail_info") != obj.end())
                info.thumbnail_info = obj.at("thumbnail_info").get<ThumbnailInfo>();
}

void
to_json(json &obj, const FileInfo &info)
{
        obj["size"]           = info.size;
        obj["mimetype"]       = info.mimetype;
        obj["thumbnail_url"]  = info.thumbnail_url;
        obj["thumbnail_info"] = info.thumbnail_info;
}

void
from_json(const json &obj, AudioInfo &info)
{
        if (obj.find("duration") != obj.end())
                info.duration = obj.at("duration").get<uint64_t>();

        if (obj.find("size") != obj.end())
                info.size = obj.at("size").get<uint64_t>();

        if (obj.find("mimetype") != obj.end())
                info.mimetype = obj.at("mimetype").get<std::string>();
}

void
to_json(json &obj, const AudioInfo &info)
{
        obj["size"]     = info.size;
        obj["duration"] = info.duration;
        obj["mimetype"] = info.mimetype;
}

void
from_json(const json &obj, VideoInfo &info)
{
        if (obj.find("w") != obj.end())
                info.w = obj.at("w").get<uint64_t>();

        if (obj.find("h") != obj.end())
                info.h = obj.at("h").get<uint64_t>();

        if (obj.find("size") != obj.end())
                info.size = obj.at("size").get<uint64_t>();

        if (obj.find("duration") != obj.end())
                info.duration = obj.at("duration").get<uint64_t>();

        if (obj.find("mimetype") != obj.end())
                info.mimetype = obj.at("mimetype").get<std::string>();

        if (obj.find("thumbnail_url") != obj.end())
                info.thumbnail_url = obj.at("thumbnail_url").get<std::string>();

        if (obj.find("thumbnail_info") != obj.end())
                info.thumbnail_info = obj.at("thumbnail_info").get<ThumbnailInfo>();
}

void
to_json(json &obj, const VideoInfo &info)
{
        obj["size"]           = info.size;
        obj["h"]              = info.h;
        obj["w"]              = info.w;
        obj["duration"]       = info.duration;
        obj["thumbnail_url"]  = info.thumbnail_url;
        obj["thumbnail_info"] = info.thumbnail_info;
        obj["mimetype"]       = info.mimetype;
}

} // namespace common
} // namespace mtx
