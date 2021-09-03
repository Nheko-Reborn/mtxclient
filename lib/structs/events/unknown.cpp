#include "mtx/events/unknown.hpp"

#include <nlohmann/json.hpp>

using json = nlohmann::json;

namespace mtx {
namespace events {

void
from_json(const json &obj, Unknown &event)
{
    event.content = obj.dump();
}

void
to_json(json &obj, const Unknown &event)
{
    try {
        obj = json::parse(event.content);
    } catch (...) {
    }
}

} // namespace events
} // namespace mtx
