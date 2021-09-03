#include "mtx/responses/turn_server.hpp"

#include <nlohmann/json.hpp>

namespace mtx::responses {

void
from_json(const nlohmann::json &obj, TurnServer &turnServer)
{
    turnServer.username = obj.at("username").get<std::string>();
    turnServer.password = obj.at("password").get<std::string>();
    turnServer.uris     = obj.at("uris").get<std::vector<std::string>>();
    turnServer.ttl      = obj.at("ttl").get<uint32_t>();
}
}
