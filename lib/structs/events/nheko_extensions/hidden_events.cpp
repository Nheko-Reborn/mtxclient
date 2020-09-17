#include "mtx/events/nheko_extensions/hidden_events.hpp"

namespace mtx {
namespace events {
namespace account_data {
namespace nheko_extensions {

void
from_json(const nlohmann::json &obj, HiddenEvents &content)
{
        for (const auto &typeStr : obj.at("hidden_event_types")) {
                if (auto type = getEventType(typeStr.get<std::string>());
                    type != EventType::Unsupported)
                        content.hidden_event_types.push_back(type);
        }
}

void
to_json(nlohmann::json &obj, const HiddenEvents &content)
{
        for (auto type : content.hidden_event_types) {
                obj["hidden_event_types"].push_back(to_string(type));
        }
}
}
} // namespace account_data
} // namespace events
} // namespace mtx
