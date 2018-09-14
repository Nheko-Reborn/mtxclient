#pragma once

#include <nlohmann/json.hpp>
#include <string>

using json = nlohmann::json;

namespace mtx {
namespace events {
namespace state {

struct Name
{
        std::string name;
};

void
from_json(const json &obj, Name &event);

void
to_json(json &obj, const Name &event);

} // namespace state
} // namespace events
} // namespace mtx
