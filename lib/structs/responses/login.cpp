#include "mtx/responses/login.hpp"

#include <nlohmann/json.hpp>

namespace mtx {
namespace responses {

void
from_json(const nlohmann::json &obj, Login &response)
{
    using namespace mtx::identifiers;
    response.user_id = obj.at("user_id").get<User>();

    response.access_token = obj.at("access_token").get<std::string>();

    if (obj.count("device_id") != 0)
        response.device_id = obj.at("device_id").get<std::string>();

    if (obj.count("well_known") != 0 && obj.at("well_known").is_object())
        response.well_known = obj.at("well_known").get<WellKnown>();
}

void
from_json(const nlohmann::json &obj, LoginFlow &response)
{
    response.type = obj.at("type").get<std::string>();
}
void
from_json(const nlohmann::json &obj, LoginFlows &response)
{
    response.flows = obj.at("flows").get<std::vector<LoginFlow>>();
}
}
}
