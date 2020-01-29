#include "mtx/responses/crypto.hpp"

#include <nlohmann/json.hpp>

namespace mtx {
namespace responses {

void
from_json(const nlohmann::json &obj, UploadKeys &response)
{
        response.one_time_key_counts =
          obj.at("one_time_key_counts").get<std::map<std::string, uint32_t>>();
}

void
from_json(const nlohmann::json &obj, QueryKeys &response)
{
        response.failures    = obj.at("failures").get<std::map<std::string, nlohmann::json>>();
        response.device_keys = obj.at("device_keys").get<std::map<std::string, DeviceToKeysMap>>();
}

void
from_json(const nlohmann::json &obj, ClaimKeys &response)
{
        response.failures = obj.at("failures").get<std::map<std::string, nlohmann::json>>();
        response.one_time_keys =
          obj.at("one_time_keys")
            .get<std::map<std::string, std::map<std::string, nlohmann::json>>>();
}

void
from_json(const nlohmann::json &obj, KeyChanges &response)
{
        response.changed = obj.at("changed").get<std::vector<std::string>>();
        response.left    = obj.at("left").get<std::vector<std::string>>();
}
}
}
