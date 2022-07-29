#pragma once

/// @file
/// @brief A file sent in a room.

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

//! Content of `m.room.message` with msgtype `m.file`.
struct File
{
    /// @brief A human-readable description of the file.
    ///
    /// This is recommended to be the filename of the original upload.
    std::string body;
    /// @brief The original filename of the uploaded file.
    ///
    /// SPEC_BUG: The filename is not really required.
    std::string filename;
    //! Must be 'm.file'.
    std::string msgtype;
    //! The matrix URL of the file.
    std::string url;
    //! Information about the file referred to in the url.
    mtx::common::FileInfo info;
    //! Encryption members. If present, they replace url.
    std::optional<crypto::EncryptedFile> file;
    //! Relates to for rich replies
    mtx::common::Relations relations;

    friend void from_json(const nlohmann::json &obj, File &content);
    friend void to_json(nlohmann::json &obj, const File &content);
};

} // namespace msg
} // namespace events
} // namespace mtx
