#include "mtx/responses/register.hpp"

using json = nlohmann::json;

namespace mtx {
namespace responses {

void
from_json(const json &obj, Register &response)
{
        using namespace mtx::identifiers;
        response.user_id      = obj.at("user_id").get<User>();
        response.access_token = obj.at("access_token").get<std::string>();
        response.device_id    = obj.at("device_id").get<std::string>();
}

void
from_json(const json &obj, Flow &response)
{
        response.stages = obj.at("stages").get<std::vector<std::string>>();
}

void
from_json(const json &obj, RegistrationFlows &response)
{
        response.flows   = obj.at("flows").get<std::vector<Flow>>();
        response.session = obj.at("session").get<std::string>();
}
}
}
