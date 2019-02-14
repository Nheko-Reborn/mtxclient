#pragma once

#include <nlohmann/json.hpp>
#include <string>

using json = nlohmann::json;

namespace mtx {
namespace events {
namespace account_data {

//! Content for the `m.tag` room account_data event.
//! A tag is a short string a client can attach to a room for sorting or advanced functionality.
struct Tag
{
        //! The tag list.
        //! A tag can have arbitrary JSON data attached
        std::map<std::string, json> tags;
};

void
from_json(const json &obj, Tag &content);

void
to_json(json &obj, const Tag &content);

} // namespace account_data
} // namespace events
} // namespace mtx