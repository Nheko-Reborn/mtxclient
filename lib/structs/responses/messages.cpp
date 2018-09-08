#include "mtx/responses/messages.hpp"
#include "mtx/responses/common.hpp"

using json = nlohmann::json;

namespace mtx {
namespace responses {

void
from_json(const json &obj, Messages &messages)
{
        messages.start = obj.at("start").get<std::string>();
        messages.end   = obj.at("end").get<std::string>();

        utils::parse_timeline_events(obj.at("chunk"), messages.chunk);
}
}
}
