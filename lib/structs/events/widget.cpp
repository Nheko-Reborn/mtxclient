#include "mtx/events/widget.hpp"

#include <nlohmann/json.hpp>

namespace mtx::events::state {
void
from_json(const nlohmann::json &obj, Widget &create)
{
    create.waitForIframeLoad = obj.value("waitForIframeLoad", true);
    create.type              = obj.value("type", "");
    create.url               = obj.value("url", "");
    create.name              = obj.value("name", "");
    create.id                = obj.value("id", "");
    create.creatorUserId     = obj.value("creatorUserId", "");
    create.data              = obj.value("data", std::map<std::string, std::string>{});
}

void
to_json(nlohmann::json &obj, const Widget &create)
{
    if (!create.name.empty())
        obj["name"] = create.name;
    if (!create.data.empty())
        obj["data"] = create.data;

    obj["type"]              = create.type;
    obj["url"]               = create.url;
    obj["id"]                = create.id;
    obj["creatorUserId"]     = create.creatorUserId;
    obj["waitForIframeLoad"] = create.waitForIframeLoad;
}
}
