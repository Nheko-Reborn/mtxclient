#include "mtx/responses/well-known.hpp"

#include <nlohmann/json.hpp>

using json = nlohmann::json;

namespace mtx {
namespace responses {

void
from_json(const json &obj, WellKnown &response)
{
    response.homeserver = obj.at("m.homeserver").get<ServerInformation>();

    if (obj.count("m.identity_server"))
        response.identity_server = obj.at("m.identity_server").get<ServerInformation>();
}

void
from_json(const json &obj, ServerInformation &response)
{
    response.base_url = obj.at("base_url").get<std::string>();
}
}
}
