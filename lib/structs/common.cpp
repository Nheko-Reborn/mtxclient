#include "mtx/common.hpp"

#include <nlohmann/json.hpp>

using json = nlohmann::json;

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

    if (obj.contains("signatures"))
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

void
from_json(const json &obj, CrossSigningKeys &res)
{
    res.user_id = obj.at("user_id").get<std::string>();
    res.usage   = obj.at("usage").get<std::vector<std::string>>();
    res.keys    = obj.at("keys").get<std::map<std::string, std::string>>();

    if (obj.contains("signatures"))
        res.signatures =
          obj.at("signatures").get<std::map<std::string, std::map<std::string, std::string>>>();
}

void
to_json(json &obj, const CrossSigningKeys &res)
{
    obj["user_id"]    = res.user_id;
    obj["usage"]      = res.usage;
    obj["keys"]       = res.keys;
    obj["signatures"] = res.signatures;
}

void
from_json(const json &obj, JWK &res)
{
    res.kty     = obj.at("kty").get<std::string>();
    res.key_ops = obj.at("key_ops").get<std::vector<std::string>>();
    res.alg     = obj.at("alg").get<std::string>();
    res.k       = obj.at("k").get<std::string>();
    res.ext     = obj.at("ext").get<bool>();
}

void
to_json(json &obj, const JWK &res)
{
    obj["kty"]     = res.kty;
    obj["key_ops"] = res.key_ops;
    obj["alg"]     = res.alg;
    obj["k"]       = res.k;
    obj["ext"]     = res.ext;
}

void
from_json(const json &obj, EncryptedFile &res)
{
    res.url    = obj.at("url").get<std::string>();
    res.key    = obj.at("key").get<JWK>();
    res.iv     = obj.at("iv").get<std::string>();
    res.hashes = obj.at("hashes").get<std::map<std::string, std::string>>();
    res.v      = obj.at("v").get<std::string>();
}

void
to_json(json &obj, const EncryptedFile &res)
{
    obj["url"]    = res.url;
    obj["key"]    = res.key;
    obj["iv"]     = res.iv;
    obj["hashes"] = res.hashes;
    obj["v"]      = res.v;
}
}
}
