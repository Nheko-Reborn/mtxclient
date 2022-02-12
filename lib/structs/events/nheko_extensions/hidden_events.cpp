#include "mtx/events/nheko_extensions/hidden_events.hpp"

#include <nlohmann/json.hpp>

namespace mtx {
namespace events {
namespace account_data {
namespace nheko_extensions {

void
from_json(const nlohmann::json &obj, HiddenEvents &content)
{
    if (obj.contains("hidden_event_types")) {
        content.hidden_event_types = std::vector<EventType>{};
        for (const auto &typeStr : obj.at("hidden_event_types")) {
            auto type = getEventType(typeStr.get<std::string>());
            content.hidden_event_types->push_back(type);
        }
    }
}

void
to_json(nlohmann::json &obj, const HiddenEvents &content)
{
    if (content.hidden_event_types) {
        for (auto type : content.hidden_event_types.value()) {
            obj["hidden_event_types"].push_back(to_string(type));
        }
    }
}
}
} // namespace account_data
} // namespace events
} // namespace mtx
