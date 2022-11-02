#pragma once

/// @file
/// @brief Empty responses returned by different endpoints.

#if __has_include(<nlohmann/json_fwd.hpp>)
#include <nlohmann/json_fwd.hpp>
#else
#include <nlohmann/json.hpp>
#endif

#include "mtx/identifiers.hpp"

namespace mtx {
namespace responses {

//! Used to represent empty responses
struct Empty
{
    friend void from_json(const nlohmann::json &, Empty &);
};

//! The response for a logout.
using Logout = Empty;
//! The response after changing your display name.
using DisplayName = Empty;
//! The response after inviting a user to a room.
using RoomInvite = Empty;
}
}
