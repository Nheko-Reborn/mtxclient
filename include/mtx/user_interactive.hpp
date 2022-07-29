#pragma once

/// @file
/// @brief Header with types for user interactive authentication

#include <string>
#include <string_view>
#include <unordered_map>
#include <variant>
#include <vector>

#if __has_include(<nlohmann/json_fwd.hpp>)
#include <nlohmann/json_fwd.hpp>
#else
#include <nlohmann/json.hpp>
#endif

namespace mtx {
//! Types and definitions for user interactive authentication.
namespace user_interactive {
//! The type of the different auth types.
using AuthType = std::string;
//! The different auth types.
namespace auth_types {
//! Password based authentication.
constexpr std::string_view password = "m.login.password";
//! Authentication using a ReCaptcha.
constexpr std::string_view recaptcha = "m.login.recaptcha";
//! Authentication using oauth2.
constexpr std::string_view oauth2 = "m.login.oauth2";
//! Authentication via email.
constexpr std::string_view email_identity = "m.login.email.identity";
//! Authentication using SMS?
constexpr std::string_view msisdn = "m.login.msisdn";
//! Token based auth.
constexpr std::string_view token = "m.login.token";
//! Single Sign On.
constexpr std::string_view sso = "m.login.sso"; // needed for /login at least
//! Placeholder used in alternative auth flows.
constexpr std::string_view dummy = "m.login.dummy";
//! Authentication by accepting a set of terms like a privacy policy.
constexpr std::string_view terms = "m.login.terms"; // see MSC1692
//! Authentication using a registration token. See MSC3231.
constexpr std::string_view registration_token = "m.login.registration_token";
}

//! A list of auth types
using Stages = std::vector<AuthType>;
//! A flow composed of a list of stages
struct Flow
{
    //! The stages to complete.
    Stages stages;

    friend void from_json(const nlohmann::json &obj, Flow &flow);
};

//! Parameters for oauth2.
struct OAuth2Params
{
    //! The oauth uri
    std::string uri;

    friend void from_json(const nlohmann::json &obj, OAuth2Params &params);
};

//! The desciption of one policy in the terms and conditions.
struct PolicyDescription
{
    std::string name; //!< language specific name
    std::string url;  //!< language specific link

    friend void from_json(const nlohmann::json &obj, PolicyDescription &desc);
};

//! A policy in the terms and conditions.
struct Policy
{
    //! Version of this policy
    std::string version;
    /// @brief 2 letter language code to policy name and link, fallback to "en"
    /// recommended, when language not available.
    std::unordered_map<std::string, PolicyDescription> langToPolicy;

    friend void from_json(const nlohmann::json &obj, Policy &policy);
};

//! Parameters for the auth stage requiring you to accept terms and conditions.
struct TermsParams
{
    //! The different policies by name.
    std::unordered_map<std::string, Policy> policies;

    friend void from_json(const nlohmann::json &obj, TermsParams &params);
};

//! All the different parameters.
using Params = std::variant<OAuth2Params, TermsParams, std::string>;

//! The struct returned on requests failing with 401.
struct Unauthorized
{
    // completed stages
    Stages completed;

    // session key to provide to further auth stages
    std::string session;

    // list of flows, which can be used to complete the UI auth
    std::vector<Flow> flows;

    // AuthType may be an undocumented string, not defined in auth_types
    std::unordered_map<AuthType, Params> params;

    friend void from_json(const nlohmann::json &obj, Unauthorized &unauthorized);
};

//! namespace for the request types in the different auth stages.
namespace auth {
//! Password stage
struct Password
{
    //! The password set by the user.
    std::string password;

    //! Types of identifiers.
    enum IdType
    {
        UserId,      //!< Use the identifier_user
        ThirdPartyId //!< use identifier_medium and identifier_address
    };
    //! If a user or third party identifier is used.
    IdType identifier_type;

    //! for user
    std::string identifier_user;

    //! for third party
    std::string identifier_medium;
    std::string identifier_address;
};

//! ReCaptcha stage.
struct ReCaptcha
{
    //! The recaptcha response
    std::string response;
};

//! Token stage.
struct Token
{
    //! the obtained token
    std::string token;
    //! Client generated nonce
    std::string txn_id;
};

//! Third party identifier for Email or MSISDN stages
struct ThreePIDCred
{
    //! identity server session id
    std::string sid;
    //! identity server client secret
    std::string client_secret;
    //! url of identity server authed with, e.g. 'matrix.org:8090'
    std::string id_server;
    //! access token previously registered with the identity server
    std::string id_access_token;
};

//! Email authentication stage.
struct EmailIdentity
{
    //! The 3rd party id
    //! See https://github.com/matrix-org/matrix-doc/pull/3471 for context.
    ThreePIDCred threepidCred;
};

//! SMS authentication stage.
struct MSISDN
{
    //! The 3rd party id
    //! See https://github.com/matrix-org/matrix-doc/pull/3471 for context.
    ThreePIDCred threepidCred;
};

//! Registration token authentication stage.
struct RegistrationToken
{
    //! The registration token to use
    std::string token;
};

//! OAuth2, client retries with the session only, so I'm guessing this is empty?
struct OAuth2
{};
//! Empty struct, when parameters are accepted.
struct Terms
{};
//! Empty struct to complete SSO.
struct SSO
{};
//! Empty struct to complete dummy auth.
struct Dummy
{};
//! Fallback auth.
struct Fallback
{};
}

//! The auth request to complete a stage.
struct Auth
{
    //! the session id
    std::string session;

    //! the content, depends on type
    std::variant<auth::Password,
                 auth::ReCaptcha,
                 auth::Token,
                 auth::EmailIdentity,
                 auth::MSISDN,
                 auth::OAuth2,
                 auth::Terms,
                 auth::SSO,
                 auth::Dummy,
                 auth::RegistrationToken,
                 auth::Fallback>
      content;
};
void
to_json(nlohmann::json &obj, const Auth &auth);
}
}
