#pragma once

#include <nlohmann/json.hpp>

using json = nlohmann::json;

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
};

//! Deserialization method needed by @p nlohmann::json.
void
from_json(const json &obj, ThumbnailInfo &info);

//! Serialization method needed by @p nlohmann::json.
void
to_json(json &obj, const ThumbnailInfo &info);

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
};

//! Deserialization method needed by @p nlohmann::json.
void
from_json(const json &obj, ImageInfo &info);

//! Serialization method needed by @p nlohmann::json.
void
to_json(json &obj, const ImageInfo &info);

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
};

//! Deserialization method needed by @p nlohmann::json.
void
from_json(const json &obj, FileInfo &info);

//! Serialization method needed by @p nlohmann::json.
void
to_json(json &obj, const FileInfo &info);

//! Audio clip metadata.
struct AudioInfo
{
        //! The size of the audio clip in bytes.
        uint64_t size = 0;
        //! The duration of the audio in milliseconds.
        uint64_t duration = 0;
        //! The mimetype of the audio e.g. `audio/aac`.
        std::string mimetype;
};

//! Deserialization method needed by @p nlohmann::json.
void
from_json(const json &obj, AudioInfo &info);

//! Serialization method needed by @p nlohmann::json.
void
to_json(json &obj, const AudioInfo &info);

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
};

//! Deserialization method needed by @p nlohmann::json.
void
from_json(const json &obj, VideoInfo &info);

//! Serialization method needed by @p nlohmann::json.
void
to_json(json &obj, const VideoInfo &info);

} // namespace common
} // namespace mtx
