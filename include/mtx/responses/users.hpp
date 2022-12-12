#pragma once

/// @file
/// @brief Response for the endpoint to search users

#if __has_include(<nlohmann/json_fwd.hpp>)
#include <nlohmann/json_fwd.hpp>
#else
#include <nlohmann/json.hpp>
#endif

namespace mtx {
namespace responses {

struct User
{
    //! The avatar url, as an MXC, if one exists.
    std::string avatar_url;
    //! The display name of the user, if one exists.
    std::string display_name;
    //! The user’s matrix user ID.
    std::string user_id;

    friend void from_json(const nlohmann::json &obj, User &res);
};
//! User directory search results.
struct Users
{
    //! If the search was limited by the search limit.
    bool limited;

    //! A chunk of user events.
    std::vector<User> results;

    friend void from_json(const nlohmann::json &obj, Users &res);
};
}
}
