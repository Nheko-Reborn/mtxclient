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
    if (obj.find("alias") != obj.end() && !obj.at("alias").is_null())
        canonical_alias.alias = obj.at("alias").get<std::string>();
    if (obj.contains("alt_aliases") && obj.at("alt_aliases").is_array())
        canonical_alias.alt_aliases =
          obj.at("alt_aliases").get<decltype(canonical_alias.alt_aliases)>();
}

void
to_json(json &obj, const CanonicalAlias &canonical_alias)
{
    if (!canonical_alias.alias.empty())
        obj["alias"] = canonical_alias.alias;
    if (!canonical_alias.alt_aliases.empty())
        obj["alt_aliases"] = canonical_alias.alt_aliases;
}

} // namespace state
} // namespace events
} // namespace mtx
