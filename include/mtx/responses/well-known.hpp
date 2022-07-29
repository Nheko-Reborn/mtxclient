#pragma once

/// @file
/// @brief Response for .well-known lookup.

#include <optional>

#if __has_include(<nlohmann/json_fwd.hpp>)
#include <nlohmann/json_fwd.hpp>
#else
#include <nlohmann/json.hpp>
#endif

namespace mtx {
namespace responses {
//! The info about this server.
struct ServerInformation
{
    //! Required. The base URL for client-server connections.
    std::string base_url;

    friend void from_json(const nlohmann::json &obj, ServerInformation &response);
};

//! Response from the `GET /.well-known/matrix/client` endpoint.
//! May also be returned from `POST /_matrix/client/r0/login`.
//
//! Gets discovery information about the domain
struct WellKnown
{
    //! Required. Used by clients to discover homeserver information.
    ServerInformation homeserver;
    //! Used by clients to discover identity server information.
    std::optional<ServerInformation> identity_server;

    friend void from_json(const nlohmann::json &obj, WellKnown &response);
};
}
}
