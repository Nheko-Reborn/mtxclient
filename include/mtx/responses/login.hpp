#pragma once

#include <string>

#include <nlohmann/json.hpp>

#include "mtx/identifiers.hpp"

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
        //! The hostname of the homeserver on which the account has been registered.
        std::string home_server;
        //! ID of the logged-in device.
        //! Will be the same as the corresponding parameter in the request, if one was specified.
        std::string device_id;
};

void
from_json(const nlohmann::json &obj, Login &response);
}
}
