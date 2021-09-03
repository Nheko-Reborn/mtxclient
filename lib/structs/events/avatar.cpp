#include <string>

#include <nlohmann/json.hpp>

#include "mtx/events/avatar.hpp"

using json = nlohmann::json;

namespace mtx {
namespace events {
namespace state {

void
from_json(const json &obj, Avatar &avatar)
{
    if (obj.find("info") != obj.end())
        avatar.image_info = obj.at("info").get<mtx::common::ImageInfo>();

    if (obj.find("url") != obj.end() && !obj.at("url").is_null())
        avatar.url = obj.at("url").get<std::string>();
}

void
to_json(json &obj, const Avatar &avatar)
{
    obj["info"] = avatar.image_info;
    obj["url"]  = avatar.url;
}

} // namespace state
} // namespace events
} // namespace mtx
