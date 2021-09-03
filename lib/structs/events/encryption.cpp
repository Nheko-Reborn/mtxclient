#include "mtx/events/encryption.hpp"

#include <nlohmann/json.hpp>

namespace mtx {
namespace events {
namespace state {

void
from_json(const nlohmann::json &obj, Encryption &encryption)
{
    encryption.algorithm = obj.at("algorithm").get<std::string>();

    if (obj.contains("rotation_period_ms"))
        encryption.rotation_period_ms = obj.at("rotation_period_ms").get<uint64_t>();
    if (obj.contains("rotation_period_msgs"))
        encryption.rotation_period_msgs = obj.at("rotation_period_msgs").get<uint64_t>();
}

void
to_json(nlohmann::json &obj, const Encryption &encryption)
{
    obj["algorithm"]            = encryption.algorithm;
    obj["rotation_period_ms"]   = encryption.rotation_period_ms;
    obj["rotation_period_msgs"] = encryption.rotation_period_msgs;
}

} // namespace state
} // namespace events
} // namespace mtx
