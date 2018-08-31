#include "mtx/common.hpp"

namespace mtx {
namespace crypto {

void
from_json(const json &obj, UnsignedDeviceInfo &res)
{
        if (obj.find("device_display_name") != obj.end())
                res.device_display_name = obj.at("device_display_name").get<std::string>();
}

void
to_json(json &obj, const UnsignedDeviceInfo &res)
{
        if (!res.device_display_name.empty())
                obj["device_display_name"] = res.device_display_name;
}

void
from_json(const json &obj, DeviceKeys &res)
{
        res.user_id    = obj.at("user_id").get<std::string>();
        res.device_id  = obj.at("device_id").get<std::string>();
        res.algorithms = obj.at("algorithms").get<std::vector<std::string>>();

        res.keys = obj.at("keys").get<std::map<AlgorithmDevice, std::string>>();
        res.signatures =
          obj.at("signatures").get<std::map<std::string, std::map<AlgorithmDevice, std::string>>>();

        if (obj.find("unsigned") != obj.end())
                res.unsigned_info = obj.at("unsigned").get<UnsignedDeviceInfo>();
}

void
to_json(json &obj, const DeviceKeys &res)
{
        obj["user_id"]    = res.user_id;
        obj["device_id"]  = res.device_id;
        obj["algorithms"] = res.algorithms;
        obj["keys"]       = res.keys;
        obj["signatures"] = res.signatures;

        if (!res.unsigned_info.device_display_name.empty())
                obj["unsigned"] = res.unsigned_info;
}
}
}
