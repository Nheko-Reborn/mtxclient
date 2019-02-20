#pragma once

#include <nlohmann/json.hpp>
#include <string>

using json = nlohmann::json;

namespace mtx {
namespace events {
namespace state {

struct PinnedEvents
{
        std::vector<std::string> pinned;
};

void
from_json(const json &obj, PinnedEvents &event);

void
to_json(json &obj, const PinnedEvents &event);

} // namespace state
} // namespace events
} // namespace mtx
