#include "mtx/responses/media.hpp"

#include <nlohmann/json.hpp>
#include <type_traits>

using json = nlohmann::json;

namespace mtx {
namespace responses {

void
from_json(const json &obj, ContentURI &res)
{
    res.content_uri = obj.at("content_uri").get<std::string>();
}

template<typename T>
static void
extractOptionalAttribute(const json &obj, const char *name, std::optional<T> &out)
{
    if (obj.contains(name)) {
        const auto &field = obj.at(name);
        // Matrix's OpenGraph proxying does not standardize numbers to be actual json numbers, they
        // can be (and, at least on synapse, sometimes are) strings
        if constexpr (std::is_same_v<T, std::int32_t>) {
            if (field.type() == json::value_t::string) {
                out = static_cast<T>(std::stol(field.get<std::string>()));
                return;
            }
        }
        out = field.get<T>();
    }
}

void
from_json(const json &obj, URLPreview &res)
{
    res.title = obj.at("og:title").get<std::string>();
    res.url   = obj.at("og:url").get<std::string>();
    extractOptionalAttribute(obj, "og:site_name", res.site_name);

    extractOptionalAttribute(obj, "og:image:type", res.image.type);
    extractOptionalAttribute(obj, "og:image:width", res.image.width);
    extractOptionalAttribute(obj, "og:image:height", res.image.height);
    extractOptionalAttribute(obj, "og:image:alt", res.image.alt);

    res.image.size = obj.at("matrix:image:size").get<std::uint64_t>();
    res.image.url  = obj.at("og:image").get<std::string>();

    if (obj.contains("og:description")) {
        res.description = obj.at("og:description").get<std::string>();
    }
}
}
}
