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

struct PinnedEvents
{
        std::vector<std::string> pinned;
};

void
from_json(const nlohmann::json &obj, PinnedEvents &event);

void
to_json(nlohmann::json &obj, const PinnedEvents &event);

} // namespace state
} // namespace events
} // namespace mtx
