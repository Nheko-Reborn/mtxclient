#pragma once

/// @file
/// @brief Responses returned by the media repo.

#if __has_include(<nlohmann/json_fwd.hpp>)
#include <nlohmann/json_fwd.hpp>
#else
#include <nlohmann/json.hpp>
#endif

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
}
}
