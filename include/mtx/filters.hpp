#pragma once

#if __has_include(<nlohmann/json_fwd.hpp>)
#include <nlohmann/json_fwd.hpp>
#else
#include <nlohmann/json.hpp>
#endif

#include <string>

//! Filters can be created on the server and can be
//! passed as a parameter to APIs which return events.

namespace mtx {
namespace filters {

struct Filter {
    //! A string to search for in the room metadata,
    //! e.g. name, topic, canonical alias etc. (Optional).
    std::string generic_search_term;
};

void
from_json(const nlohmann::json &obj, Filter &res);

void
to_json(nlohmann::json &obj, const Filter &res);

} // namespace filters
} // namespace mtx
