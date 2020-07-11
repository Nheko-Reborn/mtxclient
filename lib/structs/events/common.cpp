#include <nlohmann/json.hpp>

#include "mtx/events/common.hpp"

using json = nlohmann::json;

namespace {
template<class T>
T
safe_get(const json &obj, const std::string &name, T default_val = {})
try {
        return obj.value(name, default_val);
} catch (const nlohmann::json::type_error &) {
        return default_val;
}
}

namespace mtx {
namespace common {

void
from_json(const json &obj, ThumbnailInfo &info)
{
        info.h    = safe_get<uint64_t>(obj, "h");
        info.w    = safe_get<uint64_t>(obj, "w");
        info.size = safe_get<uint64_t>(obj, "size");

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
        info.h    = safe_get<uint64_t>(obj, "h");
        info.w    = safe_get<uint64_t>(obj, "w");
        info.size = safe_get<uint64_t>(obj, "size");

        if (obj.find("mimetype") != obj.end())
                info.mimetype = obj.at("mimetype").get<std::string>();

        if (obj.find("thumbnail_url") != obj.end())
                info.thumbnail_url = obj.at("thumbnail_url").get<std::string>();

        if (obj.find("thumbnail_info") != obj.end())
                info.thumbnail_info = obj.at("thumbnail_info").get<ThumbnailInfo>();

        if (obj.find("thumbnail_file") != obj.end())
                info.thumbnail_file = obj.at("thumbnail_file").get<crypto::EncryptedFile>();

        if (obj.find("xyz.amorgan.blurhash") != obj.end())
                info.blurhash = obj.at("xyz.amorgan.blurhash").get<std::string>();
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
        if (!info.blurhash.empty())
                obj["xyz.amorgan.blurhash"] = info.blurhash;
}

void
from_json(const json &obj, FileInfo &info)
{
        info.size = safe_get<uint64_t>(obj, "size");

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
        info.duration = safe_get<uint64_t>(obj, "duration");
        info.size     = safe_get<uint64_t>(obj, "size");

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
        info.h        = safe_get<uint64_t>(obj, "h");
        info.w        = safe_get<uint64_t>(obj, "w");
        info.size     = safe_get<uint64_t>(obj, "size");
        info.duration = safe_get<uint64_t>(obj, "duration");

        if (obj.find("mimetype") != obj.end())
                info.mimetype = obj.at("mimetype").get<std::string>();

        if (obj.find("thumbnail_url") != obj.end())
                info.thumbnail_url = obj.at("thumbnail_url").get<std::string>();

        if (obj.find("thumbnail_info") != obj.end())
                info.thumbnail_info = obj.at("thumbnail_info").get<ThumbnailInfo>();

        if (obj.find("thumbnail_file") != obj.end())
                info.thumbnail_file = obj.at("thumbnail_file").get<crypto::EncryptedFile>();

        if (obj.find("xyz.amorgan.blurhash") != obj.end())
                info.blurhash = obj.at("xyz.amorgan.blurhash").get<std::string>();
}

void
to_json(json &obj, const VideoInfo &info)
{
        obj["size"]     = info.size;
        obj["h"]        = info.h;
        obj["w"]        = info.w;
        obj["duration"] = info.duration;
        obj["mimetype"] = info.mimetype;
        if (!info.thumbnail_url.empty()) {
                obj["thumbnail_url"]  = info.thumbnail_url;
                obj["thumbnail_info"] = info.thumbnail_info;
        }
        if (info.thumbnail_file)
                obj["thumbnail_file"] = info.thumbnail_file.value();
        if (!info.blurhash.empty())
                obj["xyz.amorgan.blurhash"] = info.blurhash;
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
to_json(json &obj, const RelationType &type)
{
        switch (type) {
        case RelationType::Annotation:
                obj = "m.annotation";
                break;
        case RelationType::Reference:
                obj = "m.reference";
                break;
        case RelationType::Replace:
                obj = "m.replace";
                break;
        case RelationType::Unsupported:
        default:
                obj = "unsupported";
                break;
        }
}

void
from_json(const json &obj, RelationType &type)
{
        if (obj.get<std::string>() == "m.annotation")
                type = RelationType::Annotation;
        else if (obj.get<std::string>() == "m.reference")
                type = RelationType::Reference;
        else if (obj.get<std::string>() == "m.replace")
                type = RelationType::Replace;
        else
                type = RelationType::Unsupported;
}

void
from_json(const json &obj, ReactionRelatesTo &relates_to)
{
        if (obj.find("rel_type") != obj.end())
                relates_to.rel_type = obj.at("rel_type").get<RelationType>();
        if (obj.find("event_id") != obj.end())
                relates_to.event_id = obj.at("event_id").get<std::string>();
        if (obj.find("key") != obj.end())
                relates_to.key = obj.at("key").get<std::string>();
}

void
to_json(json &obj, const ReactionRelatesTo &relates_to)
{
        obj["rel_type"] = relates_to.rel_type;
        obj["event_id"] = relates_to.event_id;
        obj["key"]      = relates_to.key;
}

void
from_json(const json &obj, ReplyRelatesTo &relates_to)
{
        if (obj.find("m.in_reply_to") != obj.end())
                relates_to.in_reply_to = obj.at("m.in_reply_to").get<InReplyTo>();
}

void
to_json(json &obj, const ReplyRelatesTo &relates_to)
{
        obj["m.in_reply_to"] = relates_to.in_reply_to;
}

} // namespace common
} // namespace mtx
