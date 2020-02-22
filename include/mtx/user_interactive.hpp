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
}
