#include <nlohmann/json.hpp>

using json = nlohmann::json;

#include "mtx/events/server_acl.hpp"

namespace mtx {
namespace events {
namespace state {

void
from_json(const json &obj, ServerAcl &content)
{
    content.allow             = obj.value("allow", std::vector<std::string>{});
    content.deny              = obj.value("deny", std::vector<std::string>{});
    content.allow_ip_literals = obj.value("allow_ip_literals", true);
}

void
to_json(json &obj, const ServerAcl &content)
{
    obj["allow"]             = content.allow;
    obj["deny"]              = content.deny;
    obj["allow_ip_literals"] = content.allow_ip_literals;
}

} // namespace state
} // namespace events
} // namespace mtx
