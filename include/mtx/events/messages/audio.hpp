#pragma once

/// @file
/// @brief Audio messages.

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

//! Content of `m.room.message` with msgtype `m.audio`.
struct Audio
{
    /// @brief A description of the audio or some kind of content description
    /// for accessibility.
    std::string body;
    //! Must be 'm.audio'.
    std::string msgtype;
    //! The matrix URL of the audio clip.
    std::string url;
    //! Metadata for the audio clip referred to in url.
    mtx::common::AudioInfo info;
    //! Encryption members. If present, they replace url.
    std::optional<crypto::EncryptedFile> file;
    //! Relates to for rich replies
    mtx::common::Relations relations;

    friend void from_json(const nlohmann::json &obj, Audio &content);
    friend void to_json(nlohmann::json &obj, const Audio &content);
};

} // namespace msg
} // namespace events
} // namespace mtx
