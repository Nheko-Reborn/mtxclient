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

struct Image
{
        // A textual representation of the image. This could be
        // the alt text of the image, the filename of the image,
        // or some kind of content description for accessibility e.g. 'image attachment
        std::string body;
        // Must be 'm.image'.
        std::string msgtype;
        // The Matrix URL to the image.
        std::string url;
        // Metadata about the image referred to in `url`.
        common::ImageInfo info;
        // Encryption members. If present, they replace url.
        std::optional<crypto::EncryptedFile> file;
};

struct StickerImage
{
        // A textual representation of the image. This could be
        // the alt text of the image, the filename of the image,
        // or some kind of content description for accessibility e.g. 'image attachment
        std::string body;
        // The Matrix URL to the image.
        std::string url;
        // Metadata about the image referred to in `url`.
        common::ImageInfo info;
        // Encryption members. If present, they replace url.
        std::optional<crypto::EncryptedFile> file;
};

void
from_json(const json &obj, Image &content);

void
to_json(json &obj, const Image &content);

void
from_json(const json &obj, StickerImage &content);

void
to_json(json &obj, const StickerImage &content);

} // namespace msg
} // namespace events
} // namespace mtx
