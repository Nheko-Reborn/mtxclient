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
    case RelationType::Thread:
        obj = "m.thread";
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
    else if (obj.get<std::string>() == "im.nheko.relations.v1.in_reply_to" ||
             obj.get<std::string>() == "m.in_reply_to")
        type = RelationType::InReplyTo;
    else if (obj.get<std::string>() == "m.thread")
        type = RelationType::Thread;
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
            const auto &relates_to = content.at("m.relates_to");
            if (relates_to.contains("m.in_reply_to")) {
                Relation r;
                r.event_id = relates_to.at("m.in_reply_to").at("event_id").get<std::string>();
                r.rel_type = RelationType::InReplyTo;

                Relations rels;
                if (auto thread_type = relates_to.find("rel_type");
                    thread_type != relates_to.end() && *thread_type == "m.thread") {
                    if (auto thread_id = relates_to.find("event_id");
                        thread_id != relates_to.end()) {
                        r.is_fallback = relates_to.value("is_falling_back", false);
                        rels.relations.push_back(relates_to.get<mtx::common::Relation>());
                    }
                }

                rels.relations.push_back(r);
                rels.synthesized = true;
                return rels;
            } else {
                Relation r = relates_to.get<mtx::common::Relation>();
                Relations rels;
                rels.relations.push_back(r);
                rels.synthesized = true;

                if (r.rel_type == RelationType::Replace && content.contains("m.new_content") &&
                    content.at("m.new_content").contains("m.relates_to")) {
                    const auto secondRel = content["m.new_content"]["m.relates_to"];
                    if (secondRel.contains("m.in_reply_to")) {
                        Relation r2{};
                        r.rel_type = RelationType::InReplyTo;
                        r.event_id =
                          secondRel.at("m.in_reply_to").at("event_id").get<std::string>();
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

    std::optional<Relation> edit, not_edit, reply;
    for (const auto &r : relations.relations) {
        if (r.rel_type == RelationType::Replace)
            edit = r;
        else if (r.rel_type == RelationType::InReplyTo)
            reply = r;
        else
            not_edit = r;
    }

    if (not_edit) {
        content["m.relates_to"] = *not_edit;
    }

    if (reply) {
        content["m.relates_to"]["m.in_reply_to"]["event_id"] = reply->event_id;
        if (reply->is_fallback && not_edit && not_edit->rel_type == RelationType::Thread)
            content["m.relates_to"]["is_falling_back"] = true;
    }

    if (edit) {
        if (not_edit)
            content["m.new_content"]["m.relates_to"] = content["m.relates_to"];
        content["m.relates_to"] = *edit;
    }

    if (!relations.synthesized) {
        for (const auto &r : relations.relations) {
            if (r.rel_type != RelationType::Unsupported)
                content["im.nheko.relations.v1.relations"].push_back(r);
        }
    }
}
void
apply_relations(nlohmann::json &content, const Relations &relations)
{
    add_relations(content, relations);

    if (relations.replaces()) {
        for (const auto &e : content.items()) {
            if (e.key() != "m.relates_to" && e.key() != "im.nheko.relations.v1.relations" &&
                e.key() != "m.new_content") {
                content["m.new_content"][e.key()] = e.value();
            }
        }

        if (content.contains("body")) {
            content["body"] = "* " + content["body"].get<std::string>();
        }
        if (content.contains("formatted_body")) {
            content["formatted_body"] = "* " + content["formatted_body"].get<std::string>();
        }
    }
}

void
from_json(const json &obj, Relation &relates_to)
{
    if (auto it = obj.find("rel_type"); it != obj.end())
        relates_to.rel_type = it->get<RelationType>();
    if (auto it = obj.find("event_id"); it != obj.end())
        relates_to.event_id = it->get<std::string>();
    if (auto it = obj.find("key"); it != obj.end())
        relates_to.key = it->get<std::string>();
    if (auto it = obj.find("im.nheko.relations.v1.is_fallback"); it != obj.end())
        relates_to.is_fallback = it->get<bool>();
}

void
to_json(json &obj, const Relation &relates_to)
{
    obj["rel_type"] = relates_to.rel_type;
    obj["event_id"] = relates_to.event_id;
    if (relates_to.key.has_value())
        obj["key"] = relates_to.key.value();
    if (relates_to.is_fallback)
        obj["im.nheko.relations.v1.is_fallback"] = true;
}

static inline std::optional<std::string>
return_first_relation_matching(RelationType t, const Relations &rels, bool include_fallback)
{
    for (const auto &r : rels.relations)
        if (r.rel_type == t && (include_fallback || r.is_fallback == false))
            return r.event_id;
    return std::nullopt;
}
std::optional<std::string>
Relations::reply_to(bool include_fallback) const
{
    return return_first_relation_matching(RelationType::InReplyTo, *this, include_fallback);
}
std::optional<std::string>
Relations::replaces(bool include_fallback) const
{
    return return_first_relation_matching(RelationType::Replace, *this, include_fallback);
}
std::optional<std::string>
Relations::references(bool include_fallback) const
{
    return return_first_relation_matching(RelationType::Reference, *this, include_fallback);
}
std::optional<std::string>
Relations::thread(bool include_fallback) const
{
    return return_first_relation_matching(RelationType::Thread, *this, include_fallback);
}
std::optional<Relation>
Relations::annotates(bool include_fallback) const
{
    for (const auto &r : relations)
        if (r.rel_type == RelationType::Annotation && (include_fallback || r.is_fallback == false))
            return r;
    return std::nullopt;
}
} // namespace common
} // namespace mtx
