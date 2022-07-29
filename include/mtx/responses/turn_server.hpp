#pragma once

/// @file
/// @brief Responses for the turn server used for calls.

#include <string>
#include <vector>

#if __has_include(<nlohmann/json_fwd.hpp>)
#include <nlohmann/json_fwd.hpp>
#else
#include <nlohmann/json.hpp>
#endif

namespace mtx {
namespace responses {

//! Response of the `GET /_matrix/client/r0/voip/turnServer` endpoint.
//
//! This API provides credentials for the client to use when initiating calls.
struct TurnServer
{
    //! The username to use.
    std::string username;

    //! The password to use.
    std::string password;

    //! A list of TURN URIs.
    std::vector<std::string> uris;

    //! The time-to-live in seconds.
    uint32_t ttl;

    friend void from_json(const nlohmann::json &obj, TurnServer &turnServer);
};
}
}
