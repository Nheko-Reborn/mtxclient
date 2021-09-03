#include "mtx/responses/version.hpp"

#include <regex>

#include <nlohmann/json.hpp>

const static std::regex VERSION_REGEX("r(\\d+)\\.(\\d+)\\.(\\d+)");

namespace mtx {
namespace responses {

void
from_json(const nlohmann::json &obj, Versions &response)
{
    response.versions = obj.at("versions").get<std::vector<std::string>>();

    for (auto &version : response.versions) {
        if (!std::regex_match(version, VERSION_REGEX))
            throw std::invalid_argument(version + ": invalid version");
    }
}
}
}
