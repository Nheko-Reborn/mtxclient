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

    friend void from_json(const nlohmann::json &obj, Login &response);
};

//! Identity provider for SSO
struct IdentityProvider
{
    //! Optional UI hint for what kind of common SSO provider is being described in this IdP. Matrix
    //! maintains a registry of identifiers in the matrix-doc repo to ensure clients and servers are
    //! aligned on major/common brands.
    //!
    //! Clients should prefer the brand over the icon, when both are provided. Clients are not
    //! required to support any particular brand, including those in the registry, though are
    //! expected to be able to present any IdP based off the name/icon to the user regardless.
    //!
    //! Unregistered brands are permitted using the Common Namespaced Identifier Grammar, though
    //! excluding the namespace requirements. For example, examplesso is a valid brand which is not
    //! in the registry but still permitted. Servers should be mindful that clients might not
    //! support their unregistered brand usage as intended by the server.
    std::string brand;
    //! Optional MXC URI to provide an image/icon representing the IdP. Intended to be shown
    //! alongside the name if provided.
    std::string icon;
    //! Required: Opaque string chosen by the homeserver, uniquely identifying the IdP from other
    //! IdPs the homeserver might support. Should be between 1 and 255 characters in length,
    //! containing unreserved characters under RFC 3986 (ALPHA DIGIT "-" / "." / "_" / "~"). Clients
    //! are not intended to parse or infer meaning from opaque strings.
    std::string id;
    //! Required: Human readable description for the IdP, intended to be shown to the user.
    std::string name;

    friend void from_json(const nlohmann::json &obj, IdentityProvider &response);
};

//! One supported login flow.
struct LoginFlow
{
    //! The authentication used for this flow.
    mtx::user_interactive::AuthType type;

    //! Optional identity providers (IdPs) to present to the user. These would appear (typically) as
    //! distinct buttons for the user to interact with, and would map to the appropriate
    //! IdP-dependent redirect endpoint for that IdP.
    std::vector<IdentityProvider> identity_providers;

    friend void from_json(const nlohmann::json &obj, LoginFlow &response);
};

//! Response from the `GET /_matrix/client/r0/login` endpoint.
struct LoginFlows
{
    //! The list of supported flows.
    std::vector<LoginFlow> flows;

    friend void from_json(const nlohmann::json &obj, LoginFlows &response);
};
}
}
