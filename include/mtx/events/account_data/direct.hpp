#pragma once

/// @file
/// @brief The direct chats of a user.

#include <map>
#include <string>

#if __has_include(<nlohmann/json_fwd.hpp>)
#include <nlohmann/json_fwd.hpp>
#else
#include <nlohmann/json.hpp>
#endif

namespace mtx {
namespace events {
namespace account_data {

/// @brief The direct chats for a user / `m.direct`.
///
/// A map of which rooms are considered ‘direct’ rooms for specific users is kept in account_data in
/// an event of type m.direct. The content of this event is an object where the keys are the user
/// IDs and values are lists of room ID strings of the ‘direct’ rooms for that user ID.
struct Direct
{
    //! A map of which rooms are considered ‘direct’ rooms for specific users is kept in
    //! account_data in an event of type m.direct. The content of this event is an object where the
    //! keys are the user IDs and values are lists of room ID strings of the ‘direct’ rooms for that
    //! user ID.
    std::map<std::string, std::vector<std::string>> user_to_rooms;

    //! Deserialization method needed by @p nlohmann::json.
    friend void from_json(const nlohmann::json &obj, Direct &content);

    //! Serialization method needed by @p nlohmann::json.
    friend void to_json(nlohmann::json &obj, const Direct &content);
};

} // namespace state
} // namespace events
} // namespace mtx
