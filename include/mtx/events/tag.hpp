#pragma once

#include <nlohmann/json.hpp>
#include <string>

namespace mtx {
namespace events {
namespace account_data {

//! Content for the `m.tag` room account_data event.
//! A tag is a short string a client can attach to a room for sorting or advanced functionality.
struct Tag
{
        //! The tag list.
        //! A tag can have arbitrary JSON data attached
        std::map<std::string, nlohmann::json> tags;
};

void
from_json(const nlohmann::json &obj, Tag &content);

void
to_json(nlohmann::json &obj, const Tag &content);

} // namespace account_data
} // namespace events
} // namespace mtx
