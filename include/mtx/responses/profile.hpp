#pragma once

/// @file
/// @brief Responses from the profile API.

#if __has_include(<nlohmann/json_fwd.hpp>)
#include <nlohmann/json_fwd.hpp>
#else
#include <nlohmann/json.hpp>
#endif

namespace mtx {
namespace responses {

//! Response from the `GET /_matrix/client/r0/profile/{userId}` endpoint.
//
//! Get the combined profile information for this user.
//! This API may be used to fetch the user's own profile
//! information or other users; either locally or on remote homeservers.
//! This API may return keys which are not limited to *displayname* or *avatar_url*.
struct Profile
{
    //! The user's avatar URL if they have set one, otherwise not present.
    std::string avatar_url;
    //! The user's display name if they have set one, otherwise not present.
    std::string display_name;

    friend void from_json(const nlohmann::json &obj, Profile &profile);
};

//! Response from the `GET /_matrix/client/r0/profile/{userId}/avatar_url` endpoint.
struct AvatarUrl
{
    std::string avatar_url;

    friend void from_json(const nlohmann::json &obj, AvatarUrl &avatar);
};
}
}
