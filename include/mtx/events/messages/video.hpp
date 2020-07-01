#pragma once

#include <optional>
#include <string>

#include <nlohmann/json.hpp>

#include "mtx/common.hpp"
#include "mtx/events/common.hpp"

namespace mtx {
namespace events {
namespace msg {

struct Video
{
        // A description of the video or some kind of content description
        // for accessibility.
        std::string body;
        // Must be 'm.video'.
        std::string msgtype;
        // The matrix URL of the video clip.
        std::string url;
        // Metadata for the video clip referred to in url.
        mtx::common::VideoInfo info;
        // Encryption members. If present, they replace url.
        std::optional<crypto::EncryptedFile> file;
        //! Relates to for rich replies
        mtx::common::ReplyRelatesTo relates_to;
};

void
from_json(const nlohmann::json &obj, Video &content);

void
to_json(nlohmann::json &obj, const Video &content);

} // namespace msg
} // namespace events
} // namespace mtx
