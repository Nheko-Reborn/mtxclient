#include <nlohmann/json.hpp>
#include <string>

#include "mtx/events/common.hpp"
#include "mtx/events/messages/unknown.hpp"

using json = nlohmann::json;

namespace mtx {
namespace events {
namespace msg {

void
from_json(const json &obj, Unknown &content)
{
    content.content   = obj.dump();
    content.body      = obj.at("body").get<std::string>();
    content.msgtype   = obj.at("msgtype").get<std::string>();
    content.relations = common::parse_relations(obj);
}

void
to_json(json &obj, const Unknown &content)
{
    if (!content.content.empty())
        obj = json::parse(content.content);
    obj["msgtype"] = content.msgtype;
    obj["body"]    = content.body;
    common::apply_relations(obj, content.relations);
}

} // namespace msg
} // namespace events
} // namespace mtx
