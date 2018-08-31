#pragma once

#include <string>

#include <json.hpp>

#include "mtx/identifiers.hpp"

namespace mtx {
namespace responses {

// Used to represent empty responses
struct Empty
{};

void
from_json(const nlohmann::json &, Empty &);

using Logout      = Empty;
using DisplayName = Empty;
using RoomInvite  = Empty;
}
}
