#include "mtx/user_interactive.hpp"

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
        policy.version = obj.at("version");

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

        u.session = obj.at("session");
        u.flows   = obj.at("flows").get<std::vector<Flow>>();

        if (obj.contains("params")) {
                for (const auto &e : obj["params"].items()) {
                        if (e.key() == auth_types::terms)
                                u.params.emplace(e.key(), e.value().get<TermsParams>());
                        else if (e.key() == auth_types::oauth2)
                                u.params.emplace(e.key(), e.value().get<OAuth2Params>());
                        else
                                u.params.emplace(e.key(), e.value());
                }
        }
}
}
