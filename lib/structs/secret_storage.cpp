#include "mtx/secret_storage.hpp"

#include <nlohmann/json.hpp>

namespace mtx {
namespace secret_storage {

void
to_json(nlohmann::json &obj, const AesHmacSha2EncryptedData &data)
{
    obj["iv"]         = data.iv;
    obj["ciphertext"] = data.ciphertext;
    obj["mac"]        = data.mac;
}

void
from_json(const nlohmann::json &obj, AesHmacSha2EncryptedData &data)
{
    data.iv         = obj.at("iv");
    data.ciphertext = obj.at("ciphertext");
    data.mac        = obj.at("mac");
}

void
to_json(nlohmann::json &obj, const Secret &secret)
{
    obj["encrypted"] = secret.encrypted;
}

void
from_json(const nlohmann::json &obj, Secret &secret)
{
    secret.encrypted = obj.at("encrypted").get<decltype(secret.encrypted)>();
}

void
to_json(nlohmann::json &obj, const PBKDF2 &desc)
{
    obj["algorithm"]  = desc.algorithm;
    obj["salt"]       = desc.salt;
    obj["iterations"] = desc.iterations;
    obj["bits"]       = desc.bits;
}

void
from_json(const nlohmann::json &obj, PBKDF2 &desc)
{
    desc.algorithm  = obj.at("algorithm");
    desc.salt       = obj.at("salt");
    desc.iterations = obj.at("iterations");
    desc.bits       = obj.value("bits", 256);
}

void
to_json(nlohmann::json &obj, const AesHmacSha2KeyDescription &desc)
{
    obj["name"]      = desc.name;
    obj["algorithm"] = desc.algorithm;

    if (desc.passphrase)
        obj["passphrase"] = desc.passphrase.value();
    if (!desc.iv.empty())
        obj["iv"] = desc.iv;
    if (!desc.mac.empty())
        obj["mac"] = desc.mac;

    if (!desc.signatures.empty())
        obj["signatures"] = desc.signatures;
}

void
from_json(const nlohmann::json &obj, AesHmacSha2KeyDescription &desc)
{
    desc.name      = obj.value("name", ""); // Riot bug, not always present
    desc.algorithm = obj.at("algorithm");

    if (obj.contains("passphrase"))
        desc.passphrase = obj["passphrase"];
    desc.iv  = obj.value("iv", "");
    desc.mac = obj.value("mac", "");

    if (obj.contains("signatures"))
        desc.signatures = obj["signatures"].get<decltype(desc.signatures)>();
}
}
}
