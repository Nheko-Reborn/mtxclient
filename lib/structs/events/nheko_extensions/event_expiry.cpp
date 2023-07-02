#include "mtx/events/nheko_extensions/event_expiry.hpp"

#include <nlohmann/json.hpp>

namespace mtx {
namespace events {
namespace account_data {
namespace nheko_extensions {

void
from_json(const nlohmann::json &obj, EventExpiry &content)
{
    content.exclude_state_events = obj.value("exclude_state_events", false);
    content.expire_after_ms      = obj.value("expire_after_ms", std::uint64_t{0});
    content.protect_latest       = obj.value("protect_latest", std::uint64_t{0});
    content.keep_only_latest     = obj.value("keep_only_latest", std::uint64_t{0});
}

void
to_json(nlohmann::json &obj, const EventExpiry &content)
{
    if (content.exclude_state_events)
        obj["exclude_state_events"] = true;
    if (content.expire_after_ms)
        obj["expire_after_ms"] = content.expire_after_ms;
    if (content.protect_latest)
        obj["protect_latest"] = content.protect_latest;
    if (content.keep_only_latest)
        obj["keep_only_latest"] = content.keep_only_latest;
}
}
} // namespace account_data
} // namespace events
} // namespace mtx

