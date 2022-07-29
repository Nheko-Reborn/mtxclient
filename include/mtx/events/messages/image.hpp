#pragma once

/// @file
/// @brief Images sent in a room.

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

//! Content of `m.room.message` with msgtype `m.image`.
struct Image
{
    //! A textual representation of the image. This could be
    //! the alt text of the image, the filename of the image,
    //! or some kind of content description for accessibility e.g. 'image attachment
    std::string body;
    //! Must be 'm.image'.
    std::string msgtype;
    //! The Matrix URL to the image.
    std::string url;
    //! Metadata about the image referred to in `url`.
    mtx::common::ImageInfo info;
    //! Encryption members. If present, they replace url.
    std::optional<crypto::EncryptedFile> file;
    //! Relates to for rich replies
    mtx::common::Relations relations;

    friend void from_json(const nlohmann::json &obj, Image &content);
    friend void to_json(nlohmann::json &obj, const Image &content);
};

//! Content of `m.sticker`.
struct StickerImage
{
    //! A textual representation of the image. This could be
    //! the alt text of the image, the filename of the image,
    //! or some kind of content description for accessibility e.g. 'image attachment
    std::string body;
    //! The Matrix URL to the image.
    std::string url;
    //! Metadata about the image referred to in `url`.
    mtx::common::ImageInfo info;
    //! Encryption members. If present, they replace url.
    std::optional<crypto::EncryptedFile> file;
    //! Relates to for rich replies
    mtx::common::Relations relations;

    friend void from_json(const nlohmann::json &obj, StickerImage &content);
    friend void to_json(nlohmann::json &obj, const StickerImage &content);
};

} // namespace msg
} // namespace events
} // namespace mtx
