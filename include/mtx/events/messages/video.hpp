#pragma once

/// @file
/// @brief A video sent in a room.

#include <optional>
#include <string>

#if __has_include(<nlohmann/json_fwd.hpp>)
#include <nlohmann/json_fwd.hpp>
#else
#include <nlohmann/json.hpp>
#endif

#include "mtx/common.hpp"
#include "mtx/events/common.hpp"

namespace mtx {
namespace events {
namespace msg {

//! Content of `m.room.message` with msgtype `m.video`.
struct Video
{
    //! The video caption or original filename.
    //!
    //! If `filename` is unset or identical, this is the original filename of the upload.
    //! Otherwise, this is a caption for the video.
    std::string body;
    //! The optional original filename of the uploaded video.
    std::string filename;
    //! Must be 'm.video'.
    std::string msgtype;
    //! The matrix URL of the video clip.
    std::string url;
    //! Metadata for the video clip referred to in url.
    mtx::common::VideoInfo info;
    //! Encryption members. If present, they replace url.
    std::optional<crypto::EncryptedFile> file;
    //! Relates to for rich replies
    mtx::common::Relations relations;

    //! Mentioned users by this event
    std::optional<mtx::common::Mentions> mentions;

    friend void from_json(const nlohmann::json &obj, Video &content);
    friend void to_json(nlohmann::json &obj, const Video &content);
};

} // namespace msg
} // namespace events
} // namespace mtx
