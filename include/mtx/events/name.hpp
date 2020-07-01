#pragma once

#include <nlohmann/json.hpp>

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
