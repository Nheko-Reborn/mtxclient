#include <nlohmann/json.hpp>
#include <string>

#include "mtx/events/common.hpp"
#include "mtx/events/messages/location.hpp"

using json = nlohmann::json;

namespace mtx {
namespace events {
namespace msg {

void
from_json(const json &obj, Location &content)
{
    content.body    = obj.at("body").get<std::string>();
    content.msgtype = obj.at("msgtype").get<std::string>();
    if (obj.find("geo_uri") != obj.end())
        content.geo_uri = obj.at("geo_uri").get<std::string>();

    if (obj.find("info") != obj.end())
        content.info = obj.at("info").get<common::LocationInfo>();

    content.relations = common::parse_relations(obj);
}

void
to_json(json &obj, const Location &content)
{
    obj["msgtype"] = "m.location";
    obj["body"]    = content.body;

    obj["geo_uri"] = content.geo_uri;
    obj["info"]    = content.info;
    common::apply_relations(obj, content.relations);
}

} // namespace msg
} // namespace events
} // namespace mtx
