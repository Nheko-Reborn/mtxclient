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
    /// @brief A description of the video or some kind of content description
    /// for accessibility.
    std::string body;
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

    friend void from_json(const nlohmann::json &obj, Video &content);
    friend void to_json(nlohmann::json &obj, const Video &content);
};

} // namespace msg
} // namespace events
} // namespace mtx
