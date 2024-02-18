#pragma once

/// @file
/// @brief Responses returned by the media repo.

#if __has_include(<nlohmann/json_fwd.hpp>)
#include <nlohmann/json_fwd.hpp>
#else
#include <nlohmann/json.hpp>
#endif

#include <optional>
#include <string>

namespace mtx {
namespace responses {

//! Represents the response of `POST /_matrix/media/r0/upload`
struct ContentURI
{
    //! The MXC URI for the uploaded content.
    std::string content_uri;

    friend void from_json(const nlohmann::json &obj, ContentURI &response);
};

// The json response is returned as prefixed fields (e.g. og:image:alt), not objects, so from_json
// would be impractical
//! Represents an OpenGraph og:image
struct OGImage
{
    //! MIME type of the media
    std::optional<std::string> type;
    //! The number of pixels wide
    std::optional<std::int32_t> width;
    //! The number of pixels high
    std::optional<std::int32_t> height;
    //! A description of the media
    std::optional<std::string> alt;
    //! mxc:// URI of the media
    std::string url;
    //! Byte size of the og:image
    std::uint64_t size;
};

//! Represents the response of `GET /_matrix/media/v3/preview_url`, see https://ogp.me
struct URLPreview
{
    //! og:title
    std::string title;
    //! og:url
    std::string url;
    //! Structure containing og:image and related fields
    OGImage image;

    //! og:description
    std::optional<std::string> description;
    //! og:site_name
    std::optional<std::string> site_name;

    friend void from_json(const nlohmann::json &obj, URLPreview &response);
};
}
}
