#pragma once

#include <boost/optional.hpp>
#include <nlohmann/json.hpp>

namespace mtx {
namespace responses {
struct ServerInformation
{
        //! Required. The base URL for client-server connections.
        std::string base_url;
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
        boost::optional<ServerInformation> identity_server;
};

void
from_json(const nlohmann::json &obj, WellKnown &response);
void
from_json(const nlohmann::json &obj, ServerInformation &response);
}
}
