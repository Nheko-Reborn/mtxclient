#pragma once

/// @file
/// @brief Structs used in multiple different event types.

#if __has_include(<nlohmann/json_fwd.hpp>)
#include <nlohmann/json_fwd.hpp>
#else
#include <nlohmann/json.hpp>
#endif

#include <optional>

#include "mtx/common.hpp"

namespace mtx {

//! Common structs used among many different event content types.
namespace common {

constexpr auto FORMAT_MSG_TYPE = "org.matrix.custom.html";

//! Metadata about an image thumbnail.
struct ThumbnailInfo
{
    //! The height of the thumbnail in pixels.
    uint64_t h = 0;
    //! The width of the thumbnail in pixels.
    uint64_t w = 0;
    //! Size of the thumbnail in bytes.
    uint64_t size = 0;
    //! The mimetype of the thumbnail, e.g. image/jpeg.
    std::string mimetype;

    //! Deserialization method needed by @p nlohmann::json.
    friend void from_json(const nlohmann::json &obj, ThumbnailInfo &info);

    //! Serialization method needed by @p nlohmann::json.
    friend void to_json(nlohmann::json &obj, const ThumbnailInfo &info);
};

//! Metadata about an image.
struct ImageInfo
{
    //! The height of the image in pixels.
    uint64_t h = 0;
    //! The width of the image in pixels.
    uint64_t w = 0;
    //! Size of the image in bytes.
    uint64_t size = 0;
    //! Metadata about the image referred to in @p thumbnail_url.
    ThumbnailInfo thumbnail_info;
    //! The URL to a thumbnail of the image.
    std::string thumbnail_url;
    //! The mimetype of the image, `e.g. image/jpeg`.
    std::string mimetype;
    //! Encryption members. If present, they replace thumbnail_url.
    std::optional<crypto::EncryptedFile> thumbnail_file;
    //! experimental blurhash, see MSC2448
    std::string blurhash;

    //! Deserialization method needed by @p nlohmann::json.
    friend void from_json(const nlohmann::json &obj, ImageInfo &info);

    //! Serialization method needed by @p nlohmann::json.
    friend void to_json(nlohmann::json &obj, const ImageInfo &info);
};

//! Metadata about a file.
struct FileInfo
{
    //! The size of the file in bytes.
    uint64_t size = 0;
    //! Metadata about the image referred to in @p thumbnail_url.
    ThumbnailInfo thumbnail_info;
    //! The URL to the thumbnail of the file.
    std::string thumbnail_url;
    //! The mimetype of the file e.g `application/pdf`.
    std::string mimetype;
    //! Encryption members. If present, they replace thumbnail_url.
    std::optional<crypto::EncryptedFile> thumbnail_file;

    //! Deserialization method needed by @p nlohmann::json.
    friend void from_json(const nlohmann::json &obj, FileInfo &info);

    //! Serialization method needed by @p nlohmann::json.
    friend void to_json(nlohmann::json &obj, const FileInfo &info);
};

//! Audio clip metadata.
struct AudioInfo
{
    //! The size of the audio clip in bytes.
    uint64_t size = 0;
    //! The duration of the audio in milliseconds.
    uint64_t duration = 0;
    //! The mimetype of the audio e.g. `audio/aac`.
    std::string mimetype;

    //! Deserialization method needed by @p nlohmann::json.
    friend void from_json(const nlohmann::json &obj, AudioInfo &info);

    //! Serialization method needed by @p nlohmann::json.
    friend void to_json(nlohmann::json &obj, const AudioInfo &info);
};

//! Video clip metadata.
struct VideoInfo
{
    //! The size of the video in bytes.
    uint64_t size = 0;
    //! The duration of the video in milliseconds.
    uint64_t duration = 0;
    //! The height of the video in pixels.
    uint64_t h = 0;
    //! The width of the video in pixels.
    uint64_t w = 0;
    //! The mimetype of the video e.g. `video/mp4`.
    std::string mimetype;
    //! The URL to an image thumbnail of the video clip.
    std::string thumbnail_url;
    //! Metadata about the image referred to in @p thumbnail_url.
    ThumbnailInfo thumbnail_info;
    //! Encryption members. If present, they replace thumbnail_url.
    std::optional<crypto::EncryptedFile> thumbnail_file;
    //! experimental blurhash, see MSC2448
    std::string blurhash;

    //! Deserialization method needed by @p nlohmann::json.
    friend void from_json(const nlohmann::json &obj, VideoInfo &info);

    //! Serialization method needed by @p nlohmann::json.
    friend void to_json(nlohmann::json &obj, const VideoInfo &info);
};

//! Location metadata
struct LocationInfo
{
    //! The URL to an image thumbnail of the video clip.
    std::string thumbnail_url;
    //! Metadata about the image referred to in @p thumbnail_url.
    ThumbnailInfo thumbnail_info;
    //! Encryption members. If present, they replace thumbnail_url.
    std::optional<crypto::EncryptedFile> thumbnail_file;
    //! experimental blurhash, see MSC2448
    std::string blurhash;

    //! Deserialization method needed by @p nlohmann::json.
    friend void from_json(const nlohmann::json &obj, ThumbnailInfo &info);

    //! Serialization method needed by @p nlohmann::json.
    friend void to_json(nlohmann::json &obj, const ThumbnailInfo &info);
};

//! Definition of rel_type for relations.
enum class RelationType
{
    //! m.annotation rel_type
    Annotation,
    //! m.reference rel_type
    Reference,
    //! m.replace rel_type
    Replace,
    //! im.nheko.relations.v1.in_reply_to rel_type
    InReplyTo,
    //! not one of the supported types
    Unsupported
};

void
from_json(const nlohmann::json &obj, RelationType &type);

void
to_json(nlohmann::json &obj, const RelationType &type);

//! Relates to for reactions
struct Relation
{
    //! Type of relation
    RelationType rel_type = RelationType::Unsupported;
    //! event id being reacted to
    std::string event_id = "";
    //! key is the reaction itself
    std::optional<std::string> key = std::nullopt;

    friend void from_json(const nlohmann::json &obj, Relation &relation);
    friend void to_json(nlohmann::json &obj, const Relation &relation);
};

//! Multiple relations for a event
struct Relations
{
    //! All the relations for this event
    std::vector<Relation> relations;
    //! Flag, if we generated this from relates_to relations or used
    //! im.nheko.relactions.v1.relations
    bool synthesized = false;

    std::optional<std::string> reply_to() const;
    std::optional<std::string> replaces() const;
    std::optional<std::string> references() const;
    std::optional<Relation> annotates() const;
};

/// @brief Parses relations from a content object
///
/// @param obj The content object of an event.
Relations
parse_relations(const nlohmann::json &obj);

/// @brief Serializes relations to a content object
///
/// @param obj The content object of an event.
void
add_relations(nlohmann::json &obj, const Relations &relations);

/// @brief Applies also all the edit rules to the event in addition to adding the relations
///
/// @param obj The content object of an event.
void
apply_relations(nlohmann::json &obj, const Relations &relations);
} // namespace common
} // namespace mtx
