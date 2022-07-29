#pragma once

/// @file
/// @brief Events describing room tags.

#include <optional>
#include <string>

#if __has_include(<nlohmann/json_fwd.hpp>)
#include <nlohmann/json_fwd.hpp>
#else
#include <nlohmann/json.hpp>
#endif

namespace mtx {
namespace events {
//! Namespace for events in account_data
namespace account_data {
//! The content of a tag.
struct Tag
{
    //! A number in a range [0,1] describing a relative position of the room under
    //! the given tag.
    std::optional<double> order;

    friend void from_json(const nlohmann::json &obj, Tag &content);
    friend void to_json(nlohmann::json &obj, const Tag &content);
};

//! Content for the `m.tag` room account_data event.
//! A tag is a short string a client can attach to a room for sorting or advanced functionality.
struct Tags
{
    //! The tags on the room and their contents.
    std::map<std::string, Tag> tags;

    friend void from_json(const nlohmann::json &obj, Tags &content);
    friend void to_json(nlohmann::json &obj, const Tags &content);
};

} // namespace account_data
} // namespace events
} // namespace mtx
