#include "mtx/responses/device.hpp"

#include <nlohmann/json.hpp>

namespace mtx {
namespace responses {

void
from_json(const nlohmann::json &obj, Device &res)
{
    res.device_id = obj.at("device_id").get<std::string>();

    // This is needed because synapse sometimes sends null instead -_-
    if (obj.contains("display_name") && obj["display_name"].is_string()) {
        res.display_name = obj.value("display_name", std::string{});
    }

    // This is needed because synapse sometimes sends null instead -_-
    if (obj.contains("last_seen_ip") && obj["last_seen_ip"].is_string()) {
        res.last_seen_ip = obj.value("last_seen_ip", std::string{});
    }

    // This is needed because synapse sometimes sends null instead -_-
    if (obj.contains("last_seen_ts") && obj["last_seen_ts"].is_number()) {
        res.last_seen_ts = obj.value("last_seen_ts", size_t{});
    }
}

void
from_json(const nlohmann::json &obj, QueryDevices &response)
{
    response.devices = obj.at("devices").get<std::vector<Device>>();
}
}
}
