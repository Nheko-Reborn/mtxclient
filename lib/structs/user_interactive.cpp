#include "mtx/user_interactive.hpp"

#include <nlohmann/json.hpp>

namespace mtx::user_interactive {
void
from_json(const nlohmann::json &obj, OAuth2Params &params)
{
    params.uri = obj.value("uri", "");
}

void
from_json(const nlohmann::json &obj, PolicyDescription &d)
{
    d.name = obj.value("name", "");
    d.url  = obj.value("url", "");
}
void
from_json(const nlohmann::json &obj, Policy &policy)
{
    policy.version = obj.at("version").get<std::string>();

    for (const auto &e : obj.items())
        if (e.key() != "version")
            policy.langToPolicy.emplace(e.key(), e.value().get<PolicyDescription>());
}

void
from_json(const nlohmann::json &obj, TermsParams &terms)
{
    terms.policies = obj["policies"].get<std::unordered_map<std::string, Policy>>();
}

void
from_json(const nlohmann::json &obj, Flow &flow)
{
    flow.stages = obj["stages"].get<Stages>();
}
void
from_json(const nlohmann::json &obj, Unauthorized &u)
{
    if (obj.contains("completed"))
        u.completed = obj.at("completed").get<Stages>();

    u.session = obj.value("session", "");
    u.flows   = obj.at("flows").get<std::vector<Flow>>();

    if (obj.contains("params")) {
        for (const auto &e : obj["params"].items()) {
            if (e.key() == auth_types::terms)
                u.params.emplace(e.key(), e.value().get<TermsParams>());
            else if (e.key() == auth_types::oauth2)
                u.params.emplace(e.key(), e.value().get<OAuth2Params>());
            else
                u.params.emplace(e.key(), e.value().dump());
        }
    }
}

namespace {
template<class... Ts>
struct overloaded : Ts...
{
    using Ts::operator()...;
};
template<class... Ts>
overloaded(Ts...) -> overloaded<Ts...>;
}

namespace auth {
static void
to_json(nlohmann::json &obj, const ThreePIDCred &cred)
{
    obj["sid"]           = cred.sid;
    obj["client_secret"] = cred.client_secret;

    if (!cred.id_server.empty()) {
        obj["id_server"]       = cred.id_server;
        obj["id_access_token"] = cred.id_access_token;
    }
}
}

void
to_json(nlohmann::json &obj, const Auth &auth)
{
    obj["session"] = auth.session;

    std::visit(overloaded{
                 [&obj](const auth::Password &password) {
                     obj["type"]     = auth_types::password;
                     obj["password"] = password.password;

                     if (password.identifier_type == auth::Password::IdType::UserId) {
                         obj["identifier"]["type"] = "m.id.user";
                         obj["identifier"]["user"] = password.identifier_user;
                     } else {
                         obj["identifier"]["type"]    = "m.id.thirdparty";
                         obj["identifier"]["medium"]  = password.identifier_medium;
                         obj["identifier"]["address"] = password.identifier_address;
                     }
                 },
                 [&obj](const auth::ReCaptcha &captcha) {
                     obj["type"]     = auth_types::recaptcha;
                     obj["response"] = captcha.response;
                 },
                 [&obj](const auth::Token &token) {
                     obj["type"]   = auth_types::token;
                     obj["token"]  = token.token;
                     obj["txn_id"] = token.txn_id;
                 },
                 [&obj](const auth::EmailIdentity &id) {
                     obj["type"]           = auth_types::email_identity;
                     obj["threepid_creds"] = id.threepidCred;
                 },
                 [&obj](const auth::MSISDN &id) {
                     obj["type"]           = auth_types::msisdn;
                     obj["threepid_creds"] = id.threepidCred;
                 },
                 [&obj](const auth::RegistrationToken &registration_token) {
                     obj["type"]  = auth_types::registration_token;
                     obj["token"] = registration_token.token;
                 },
                 [&obj](const auth::OAuth2 &) { obj["type"] = auth_types::oauth2; },
                 [&obj](const auth::SSO &) { obj["type"] = auth_types::sso; },
                 [&obj](const auth::Terms &) { obj["type"] = auth_types::terms; },
                 [&obj](const auth::Dummy &) { obj["type"] = auth_types::dummy; },
                 [](const auth::Fallback &) {},
               },
               auth.content);
}
}
