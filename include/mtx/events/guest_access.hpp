#pragma once

#include <nlohmann/json.hpp>
#include <string>

using json = nlohmann::json;

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
from_json(const json &obj, GuestAccess &guest_access);

void
to_json(json &obj, const GuestAccess &guest_access);

} // namespace state
} // namespace events
} // namespace mtx
