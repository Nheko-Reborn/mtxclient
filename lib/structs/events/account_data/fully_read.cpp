#include <nlohmann/json.hpp>

#include "mtx/events/account_data/fully_read.hpp"

namespace mtx {
namespace events {
namespace account_data {

void
from_json(const nlohmann::json &obj, FullyRead &content)
{
    content.event_id = obj.at("event_id").get<std::string>();
}

void
to_json(nlohmann::json &obj, const FullyRead &content)
{
    obj["event_id"] = content.event_id;
}

} // namespace state
} // namespace events
} // namespace mtx
