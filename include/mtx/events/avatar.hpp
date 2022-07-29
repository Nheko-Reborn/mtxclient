#pragma once

/// @file
/// @brief Room avatar events.

#if __has_include(<nlohmann/json_fwd.hpp>)
#include <nlohmann/json_fwd.hpp>
#else
#include <nlohmann/json.hpp>
#endif

#include "mtx/events/common.hpp"

namespace mtx {
namespace events {
namespace state {

//! Content of the `m.room.avatar` event.
//
//! A picture that is associated with the room.
//! This can be displayed alongside the room information.
struct Avatar
{
    //! Metadata about the image referred to in @p url.
    mtx::common::ImageInfo image_info;
    //! The URL to the image.
    std::string url;

    //! Deserialization method needed by @p nlohmann::json.
    friend void from_json(const nlohmann::json &obj, Avatar &avatar);

    //! Serialization method needed by @p nlohmann::json.
    friend void to_json(nlohmann::json &obj, const Avatar &avatar);
};

} // namespace state
} // namespace events
} // namespace mtx
