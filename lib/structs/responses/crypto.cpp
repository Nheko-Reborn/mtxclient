#include "mtx/responses/crypto.hpp"

using json = nlohmann::json;

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
}
}
