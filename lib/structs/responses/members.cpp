#include "mtx/responses/members.hpp"

#include <nlohmann/json.hpp>

#include "mtx/log.hpp"

namespace mtx {
namespace responses {

void
from_json(const nlohmann::json &obj, Members &res)
{
    if (obj.contains("chunk")) {
        for (const auto &e : obj["chunk"]) {
            try {
                mtx::events::StateEvent<mtx::events::state::Member> member = e;
                res.chunk.push_back(member);
            } catch (const std::exception &e) {
                utils::log::log_warning(
                  std::string("Failed to parse member event in members chunk: ") + e.what());
            }
        }
    }
}

}
}
