#pragma once

/// @file
/// @brief Real-world locations to send in a room.

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
struct Location
{
    //! Required. A description of the location e.g. 'Big Ben, London, UK', or some kind of
    //! content description for accessibility e.g. 'location attachment'.
    std::string body;
    // Must be 'm.location'.
    std::string msgtype;
    //! Required. A geo URI representing this location.
    std::string geo_uri;
    //! A thumbnail for this location.
    mtx::common::LocationInfo info;
    //! Relates to for rich replies
    mtx::common::Relations relations;

    friend void from_json(const nlohmann::json &obj, Location &content);
    friend void to_json(nlohmann::json &obj, const Location &content);
};

} // namespace msg
} // namespace events
} // namespace mtx
