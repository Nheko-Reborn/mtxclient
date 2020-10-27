#pragma once

#if __has_include(<nlohmann/json_fwd.hpp>)
#include <nlohmann/json_fwd.hpp>
#else
#include <nlohmann/json.hpp>
#endif

#include <string>

namespace mtx {
namespace events {
namespace state {

struct Name
{
        std::string name;
};

void
from_json(const nlohmann::json &obj, Name &event);

void
to_json(nlohmann::json &obj, const Name &event);

} // namespace state
} // namespace events
} // namespace mtx
