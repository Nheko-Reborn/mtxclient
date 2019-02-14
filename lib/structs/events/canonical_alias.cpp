#include <string>

#include <nlohmann/json.hpp>

#include "mtx/events/canonical_alias.hpp"

using json = nlohmann::json;

namespace mtx {
namespace events {
namespace state {

void
from_json(const json &obj, CanonicalAlias &canonical_alias)
{
        if (!obj.at("alias").is_null())
                canonical_alias.alias = obj.at("alias").get<std::string>();
}

void
to_json(json &obj, const CanonicalAlias &canonical_alias)
{
        obj["alias"] = canonical_alias.alias;
}

} // namespace state
} // namespace events
} // namespace mtx
