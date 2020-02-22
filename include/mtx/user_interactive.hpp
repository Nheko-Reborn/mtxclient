#pragma once

#include <string>
#include <string_view>
#include <unordered_map>
#include <variant>
#include <vector>

#include <nlohmann/json.hpp>

namespace mtx::user_interactive {
using AuthType = std::string;
namespace auth_types {
constexpr std::string_view password       = "m.login.password";
constexpr std::string_view recaptcha      = "m.login.recaptcha";
constexpr std::string_view oauth2         = "m.login.oauth2";
constexpr std::string_view email_identity = "m.login.email.identity";
constexpr std::string_view msisdn         = "m.login.msisdn";
constexpr std::string_view token          = "m.login.token";
constexpr std::string_view dummy          = "m.login.dummy";
constexpr std::string_view terms          = "m.login.terms"; // see MSC1692
}

using Stages = std::vector<AuthType>;
struct Flow
{
        Stages stages;
};
void
from_json(const nlohmann::json &obj, Flow &flow);
struct OAuth2Params
{
        std::string uri;
};
void
from_json(const nlohmann::json &obj, OAuth2Params &params);

struct PolicyDescription
{
        std::string name; // language specific name
        std::string url;  // language specific link
};
void
from_json(const nlohmann::json &obj, PolicyDescription &desc);

struct Policy
{
        std::string version;
        // 2 letter language code to policy name and link, fallback to "en"
        // recommended, when language not available.
        std::unordered_map<std::string, PolicyDescription> langToPolicy;
};
void
from_json(const nlohmann::json &obj, Policy &policy);

struct TermsParams
{
        std::unordered_map<std::string, Policy> policies;
};
void
from_json(const nlohmann::json &obj, TermsParams &params);

using Params = std::variant<OAuth2Params, TermsParams, nlohmann::json>;

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
};
void
from_json(const nlohmann::json &obj, Unauthorized &unauthorized);

namespace auth {
struct Password
{
        std::string password;

        enum IdType
        {
                UserId,
                ThirdPartyId
        };
        IdType identifier_type;

        //! for user
        std::string identifier_user;

        //! for third party
        std::string identifier_medium;
        std::string identifier_address;
};

struct ReCaptcha
{
        //! The recaptcha response
        std::string response;
};

struct Token
{
        //! the obtained token
        std::string token;
        //! Client generated nonce
        std::string txn_id;
};

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
struct EmailIdentity
{
        // The 3rd party ids
        std::vector<ThreePIDCred> threepidCreds;
};
struct MSISDN
{
        // The 3rd party ids
        std::vector<ThreePIDCred> threepidCreds;
};

//! OAuth2, client retries with the session only, so I'm guessing this is empty?
struct OAuth2
{};
struct Terms
{};
struct Dummy
{};
struct Fallback
{};
}
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
                     auth::Dummy,
                     auth::Fallback>
          content;
};
void
to_json(nlohmann::json &obj, const Auth &auth);
}
