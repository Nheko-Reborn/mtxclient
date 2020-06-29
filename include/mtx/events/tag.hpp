#pragma once

#include <optional>
#include <string>

#include <nlohmann/json.hpp>

using json = nlohmann::json;

namespace mtx {
namespace events {
namespace account_data {
struct Tag
{
        //! A number in a range [0,1] describing a relative position of the room under
        //! the given tag.
        std::optional<double> order;
};
void
from_json(const json &obj, Tag &content);

void
to_json(json &obj, const Tag &content);

//! Content for the `m.tag` room account_data event.
//! A tag is a short string a client can attach to a room for sorting or advanced functionality.
struct Tags
{
        //! The tags on the room and their contents.
        std::map<std::string, Tag> tags;
};

void
from_json(const json &obj, Tags &content);

void
to_json(json &obj, const Tags &content);

} // namespace account_data
} // namespace events
} // namespace mtx
