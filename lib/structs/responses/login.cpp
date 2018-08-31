#include "mtx/responses/login.hpp"

using json = nlohmann::json;

namespace mtx {
namespace responses {

void
from_json(const json &obj, Login &response)
{
        using namespace mtx::identifiers;
        response.user_id = obj.at("user_id").get<User>();

        response.access_token = obj.at("access_token").get<std::string>();
        response.home_server  = obj.at("home_server").get<std::string>();

        if (obj.count("device_id") != 0)
                response.device_id = obj.at("device_id").get<std::string>();
}
}
}
