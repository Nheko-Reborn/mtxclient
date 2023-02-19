#include "mtx/responses/version.hpp"

#include <nlohmann/json.hpp>

namespace mtx {
namespace responses {

void
from_json(const nlohmann::json &obj, Versions &response)
{
    response.versions = obj.at("versions").get<std::vector<std::string>>();

    for (auto &version : response.versions) {
        if (version.empty())
            throw std::invalid_argument(version + ": invalid version");
    }
}
}
}
