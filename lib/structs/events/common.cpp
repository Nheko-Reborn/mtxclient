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

        if (obj.find("thumbnail_file") != obj.end())
                info.thumbnail_file = obj.at("thumbnail_file").get<crypto::EncryptedFile>();
}

void
to_json(json &obj, const ImageInfo &info)
{
        obj["h"]        = info.h;
        obj["w"]        = info.w;
        obj["size"]     = info.size;
        obj["mimetype"] = info.mimetype;
        if (!info.thumbnail_url.empty()) {
                obj["thumbnail_url"]  = info.thumbnail_url;
                obj["thumbnail_info"] = info.thumbnail_info;
        }
        if (info.thumbnail_file)
                obj["thumbnail_file"] = info.thumbnail_file.value();
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

        if (obj.find("thumbnail_file") != obj.end())
                info.thumbnail_file = obj.at("thumbnail_file").get<crypto::EncryptedFile>();
}

void
to_json(json &obj, const FileInfo &info)
{
        obj["size"]     = info.size;
        obj["mimetype"] = info.mimetype;
        if (!info.thumbnail_url.empty()) {
                obj["thumbnail_url"]  = info.thumbnail_url;
                obj["thumbnail_info"] = info.thumbnail_info;
        }
        if (info.thumbnail_file)
                obj["thumbnail_file"] = info.thumbnail_file.value();
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

        if (obj.find("thumbnail_file") != obj.end())
                info.thumbnail_file = obj.at("thumbnail_file").get<crypto::EncryptedFile>();
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
        if (info.thumbnail_file)
                obj["thumbnail_file"] = info.thumbnail_file.value();
}

void
from_json(const json &obj, InReplyTo &in_reply_to)
{
        if (obj.find("event_id") != obj.end())
                in_reply_to.event_id = obj.at("event_id").get<std::string>();
}

void
to_json(json &obj, const InReplyTo &in_reply_to)
{
        obj["event_id"] = in_reply_to.event_id;
}

void
from_json(const json &obj, RelatesTo &relates_to)
{
        if (obj.find("m.in_reply_to") != obj.end())
                relates_to.in_reply_to = obj.at("m.in_reply_to").get<InReplyTo>();
}

void
to_json(json &obj, const RelatesTo &relates_to)
{
        obj["m.in_reply_to"] = relates_to.in_reply_to;
}

} // namespace common
} // namespace mtx
