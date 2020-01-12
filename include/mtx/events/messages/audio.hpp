#pragma once

#include <optional>
#include <string>

#include <nlohmann/json.hpp>

#include "mtx/common.hpp"
#include "mtx/events/common.hpp"

using json = nlohmann::json;

namespace common = mtx::common;

namespace mtx {
namespace events {
namespace msg {

struct Audio
{
        // A description of the audio or some kind of content description
        // for accessibility.
        std::string body;
        // Must be 'm.audio'.
        std::string msgtype;
        // The matrix URL of the audio clip.
        std::string url;
        // Metadata for the audio clip referred to in url.
        common::AudioInfo info;
        // Encryption members. If present, they replace url.
        std::optional<crypto::EncryptedFile> file;
        //! Relates to for rich replies
        common::RelatesTo relates_to;
};

void
from_json(const json &obj, Audio &content);

void
to_json(json &obj, const Audio &content);

} // namespace msg
} // namespace events
} // namespace mtx
