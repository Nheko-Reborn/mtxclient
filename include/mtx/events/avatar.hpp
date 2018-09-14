#pragma once

#include <nlohmann/json.hpp>

using json = nlohmann::json;

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
};

//! Deserialization method needed by @p nlohmann::json.
void
from_json(const json &obj, Avatar &avatar);

//! Serialization method needed by @p nlohmann::json.
void
to_json(json &obj, const Avatar &avatar);

} // namespace state
} // namespace events
} // namespace mtx
