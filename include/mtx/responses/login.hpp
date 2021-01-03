#pragma once

/// @file
/// @brief Login related responses.

#include <optional>
#include <string>

#if __has_include(<nlohmann/json_fwd.hpp>)
#include <nlohmann/json_fwd.hpp>
#else
#include <nlohmann/json.hpp>
#endif

#include "mtx/identifiers.hpp"
#include "mtx/user_interactive.hpp"
#include "well-known.hpp"

namespace mtx {
namespace responses {

//! Response from the `POST /_matrix/client/r0/login` endpoint.
struct Login
{
        //! The fully-qualified Matrix ID that has been registered.
        mtx::identifiers::User user_id;
        //! An access token for the account.
        //! This access token can then be used to authorize other requests.
        std::string access_token;
        //! ID of the logged-in device.
        //! Will be the same as the corresponding parameter in the request, if one was specified.
        std::string device_id;

        //! Optional client configuration provided by the server.
        //! If present, clients SHOULD use the provided object to reconfigure themselves,
        //! optionally validating the URLs within.
        std::optional<WellKnown> well_known;
};

void
from_json(const nlohmann::json &obj, Login &response);

//! One supported login flow.
struct LoginFlow
{
        //! The authentication used for this flow.
        mtx::user_interactive::AuthType type;
};
void
from_json(const nlohmann::json &obj, LoginFlow &response);

//! Response from the `GET /_matrix/client/r0/login` endpoint.
struct LoginFlows
{
        //! The list of supported flows.
        std::vector<LoginFlow> flows;
};

void
from_json(const nlohmann::json &obj, LoginFlows &response);
}
}
