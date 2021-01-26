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
        case RelationType::InReplyTo:
                obj = "im.nheko.relations.v1.in_reply_to";
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
        else if (obj.get<std::string>() == "im.nheko.relations.v1.in_reply_to")
                type = RelationType::InReplyTo;
        else
                type = RelationType::Unsupported;
}

Relations
parse_relations(const nlohmann::json &content)
{
        try {
                if (content.contains("im.nheko.relations.v1.relations")) {
                        Relations rels;
                        rels.relations = content.at("im.nheko.relations.v1.relations")
                                           .get<std::vector<mtx::common::Relation>>();
                        rels.synthesized = false;
                        return rels;
                } else if (content.contains("m.relates_to")) {
                        if (content.at("m.relates_to").contains("m.in_reply_to")) {
                                Relation r;
                                r.event_id = content.at("m.relates_to")
                                               .at("m.in_reply_to")
                                               .at("event_id")
                                               .get<std::string>();
                                r.rel_type = RelationType::InReplyTo;

                                Relations rels;
                                rels.relations.push_back(r);
                                rels.synthesized = true;
                                return rels;
                        } else {
                                Relation r =
                                  content.at("m.relates_to").get<mtx::common::Relation>();
                                Relations rels;
                                rels.relations.push_back(r);
                                rels.synthesized = true;

                                if (r.rel_type == RelationType::Replace &&
                                    content.contains("m.new_content") &&
                                    content.at("m.new_content").contains("m.relates_to")) {
                                        const auto secondRel =
                                          content["m.new_content"]["m.relates_to"];
                                        if (secondRel.contains("m.in_reply_to")) {
                                                Relation r2{};
                                                r.rel_type = RelationType::InReplyTo;
                                                r.event_id = secondRel.at("m.in_reply_to")
                                                               .at("event_id")
                                                               .get<std::string>();
                                                rels.relations.push_back(r2);
                                        } else {
                                                rels.relations.push_back(secondRel.get<Relation>());
                                        }
                                }

                                return rels;
                        }
                }
        } catch (nlohmann::json &e) {
        }
        return {};
}

void
add_relations(nlohmann::json &content, const Relations &relations)
{
        if (relations.relations.empty())
                return;

        std::optional<Relation> edit, not_edit;
        for (const auto &r : relations.relations) {
                if (r.rel_type == RelationType::Replace)
                        edit = r;
                else
                        not_edit = r;
        }

        if (not_edit) {
                if (not_edit->rel_type == RelationType::InReplyTo) {
                        content["m.relates_to"]["m.in_reply_to"]["event_id"] = not_edit->event_id;
                } else {
                        content["m.relates_to"] = *not_edit;
                }
        }

        if (edit) {
                auto new_content         = content;
                content["m.new_content"] = new_content;
                content["m.relates_to"]  = *edit;

                if (content.contains("body"))
                        content["body"] = "* " + content["body"].get<std::string>();
                if (content.contains("formatted_body"))
                        content["formatted_body"] =
                          "* " + content["formatted_body"].get<std::string>();
        }

        if (!relations.synthesized) {
                for (const auto &r : relations.relations) {
                        if (r.rel_type != RelationType::Unsupported)
                                content["im.nheko.relations.v1.relations"].push_back(r);
                }
        }
}

void
from_json(const json &obj, Relation &relates_to)
{
        if (obj.find("rel_type") != obj.end())
                relates_to.rel_type = obj.at("rel_type").get<RelationType>();
        if (obj.find("event_id") != obj.end())
                relates_to.event_id = obj.at("event_id").get<std::string>();
        if (obj.find("key") != obj.end())
                relates_to.key = obj.at("key").get<std::string>();
}

void
to_json(json &obj, const Relation &relates_to)
{
        obj["rel_type"] = relates_to.rel_type;
        obj["event_id"] = relates_to.event_id;
        if (relates_to.key.has_value())
                obj["key"] = relates_to.key.value();
}

static inline std::optional<std::string>
return_first_relation_matching(RelationType t, const Relations &rels)
{
        for (const auto &r : rels.relations)
                if (r.rel_type == t)
                        return r.event_id;
        return std::nullopt;
}
std::optional<std::string>
Relations::reply_to() const
{
        return return_first_relation_matching(RelationType::InReplyTo, *this);
}
std::optional<std::string>
Relations::replaces() const
{
        return return_first_relation_matching(RelationType::Replace, *this);
}
std::optional<std::string>
Relations::references() const
{
        return return_first_relation_matching(RelationType::Reference, *this);
}
std::optional<Relation>
Relations::annotates() const
{
        for (const auto &r : relations)
                if (r.rel_type == RelationType::Annotation)
                        return r;
        return std::nullopt;
}
} // namespace common
} // namespace mtx
