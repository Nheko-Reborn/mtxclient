#pragma once

#if __has_include(<nlohmann/json_fwd.hpp>)
#include <nlohmann/json_fwd.hpp>
#else
#include <nlohmann/json.hpp>
#endif

#include <string>

namespace mtx {
namespace events {
namespace state {

enum class AccessState
{
        CanJoin,
        Forbidden,
};

//! Converts @p AccessState to @p std::string for serialization.
std::string
accessStateToString(AccessState state);

//! Converts @p std::string to @p AccessState for deserialization.
AccessState
stringToAccessState(const std::string &state);

//! Content of the `m.room.guest_access` state event.
struct GuestAccess
{
        //! Whether guests can join the room.
        AccessState guest_access = AccessState::Forbidden;
};

void
from_json(const nlohmann::json &obj, GuestAccess &guest_access);

void
to_json(nlohmann::json &obj, const GuestAccess &guest_access);

} // namespace state
} // namespace events
} // namespace mtx
