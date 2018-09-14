#pragma once

#include <nlohmann/json.hpp>
#include <string>

#include "mtx/events/common.hpp"

using json = nlohmann::json;

namespace common = mtx::common;

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
        common::VideoInfo info;
};

void
from_json(const json &obj, Video &content);

void
to_json(json &obj, const Video &content);

} // namespace msg
} // namespace events
} // namespace mtx
