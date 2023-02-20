#include "mtxclient/http/client.hpp"
#include "mtx/log.hpp"
#include "mtxclient/http/client_impl.hpp"

#include <mutex>
#include <thread>

#include <nlohmann/json.hpp>

#include <coeurl/client.hpp>
#include <coeurl/request.hpp>
#include <utility>

#include "mtxclient/utils.hpp"

#include "mtx/log.hpp"
#include "mtx/requests.hpp"
#include "mtx/responses.hpp"

using namespace mtx::http;

namespace mtx::http {
struct ClientPrivate
{
    coeurl::Client client;
};

void
UIAHandler::next(const user_interactive::Auth &auth) const
{
    next_(*this, auth);
}
}

Client::Client(const std::string &server, uint16_t port)
  : p{new ClientPrivate}
{
    set_server(server);
    set_port(port);

    p->client.set_verify_peer(true);
    p->client.connection_timeout(60);
}

// call destuctor of work queue and ios first!
Client::~Client() { p.reset(); }

void
Client::shutdown()
{
    p->client.shutdown();
}

void
Client::alt_svc_cache_path(const std::string &path)
{
    p->client.alt_svc_cache_path(path);
}

coeurl::Headers
mtx::http::Client::prepare_headers(bool requires_auth)
{
    coeurl::Headers headers;
    headers["User-Agent"] = "mtxclient v0.9.2";

    if (requires_auth && !access_token_.empty())
        headers["Authorization"] = "Bearer " + access_token();

    return headers;
}

std::string
mtx::http::Client::endpoint_to_url(const std::string &endpoint, const char *endpoint_namespace)
{
    return protocol_ + "://" + server_ + ":" + std::to_string(port_) + endpoint_namespace +
           endpoint;
}

void
mtx::http::Client::post(const std::string &endpoint,
                        const std::string &req,
                        mtx::http::TypeErasedCallback cb,
                        bool requires_auth,
                        const std::string &content_type)
{
    p->client.post(
      endpoint_to_url(endpoint),
      req,
      content_type,
      [cb = std::move(cb)](const coeurl::Request &r) {
          cb(r.response_headers(), r.response(), r.error_code(), r.response_code());
      },
      prepare_headers(requires_auth));
}

void
mtx::http::Client::delete_(const std::string &endpoint, ErrCallback cb, bool requires_auth)
{
    p->client.delete_(
      endpoint_to_url(endpoint),
      [cb = std::move(cb)](const coeurl::Request &r) {
          mtx::http::ClientError client_error;
          if (r.error_code()) {
              client_error.error_code = r.error_code();
              return cb(client_error);
          }

          client_error.status_code = r.response_code();

          // We only count 2xx status codes as success.
          if (client_error.status_code < 200 || client_error.status_code >= 300) {
              // The homeserver should return an error struct.
              try {
                  nlohmann::json json_error = nlohmann::json::parse(r.response());
                  client_error.matrix_error = json_error.get<mtx::errors::Error>();
              } catch (const nlohmann::json::exception &e) {
                  client_error.parse_error =
                    std::string(e.what()) + ": " + std::string(r.response());
              }
              return cb(client_error);
          }
          return cb({});
      },
      prepare_headers(requires_auth));
}

void
mtx::http::Client::put(const std::string &endpoint,
                       const std::string &req,
                       mtx::http::TypeErasedCallback cb,
                       bool requires_auth)
{
    p->client.put(
      endpoint_to_url(endpoint),
      req,
      "application/json",
      [cb = std::move(cb)](const coeurl::Request &r) {
          cb(r.response_headers(), r.response(), r.error_code(), r.response_code());
      },
      prepare_headers(requires_auth));
}

void
mtx::http::Client::get(const std::string &endpoint,
                       mtx::http::TypeErasedCallback cb,
                       bool requires_auth,
                       const std::string &endpoint_namespace,
                       int num_redirects)
{
    p->client.get(
      endpoint_to_url(endpoint, endpoint_namespace.c_str()),
      [cb = std::move(cb)](const coeurl::Request &r) {
          cb(r.response_headers(), r.response(), r.error_code(), r.response_code());
      },
      prepare_headers(requires_auth),
      num_redirects);
}

void
Client::verify_certificates(bool enabled)
{
    p->client.set_verify_peer(enabled);
}

void
Client::set_server(const std::string &server)
{
    std::string_view server_name = server;
    std::uint16_t port           = 443;
    this->protocol_              = "https";
    // Remove https prefix, if it exists
    if (server_name.substr(0, 8) == "https://") {
        server_name.remove_prefix(8);
        port = 443;
    }
    if (server_name.substr(0, 7) == "http://") {
        server_name.remove_prefix(7);
        port            = 80;
        this->protocol_ = "http";
    }
    if (server_name.size() > 0 && server_name.back() == '/')
        server_name.remove_suffix(1);

    if (std::count(server_name.begin(), server_name.end(), ':') == 1) {
        auto colon_offset = server_name.find(':');
        server_           = std::string(server_name.substr(0, colon_offset));

        auto tmp = std::string(server_name.substr(colon_offset + 1));
        if (mtx::client::utils::is_number(tmp)) {
            port_ = static_cast<std::uint16_t>(std::stoul(tmp));
            return;
        }
    }

    server_ = std::string(server_name);
    port_   = port;
}

void
Client::close(bool force)
{
    p->client.close(force);
}

//
// Client API endpoints
//

void
Client::login(const std::string &user,
              const std::string &password,
              const std::string &device_name,
              Callback<mtx::responses::Login> callback)
{
    mtx::requests::Login req;
    req.identifier                  = mtx::requests::login_identifier::User{user};
    req.password                    = password;
    req.initial_device_display_name = device_name;

    login(req, std::move(callback));
}

void
Client::login(const std::string &user,
              const std::string &password,
              Callback<mtx::responses::Login> callback)
{
    mtx::requests::Login req;
    req.identifier = mtx::requests::login_identifier::User{user};
    req.password   = password;

    login(req, std::move(callback));
}

void
Client::login(const mtx::requests::Login &req, Callback<mtx::responses::Login> callback)
{
    post<mtx::requests::Login, mtx::responses::Login>(
      "/client/v3/login",
      req,
      [_this    = shared_from_this(),
       callback = std::move(callback)](const mtx::responses::Login &resp, RequestErr err) {
          if (!err && resp.access_token.size()) {
              _this->user_id_      = resp.user_id;
              _this->device_id_    = resp.device_id;
              _this->access_token_ = resp.access_token;
          }
          callback(resp, err);
      },
      false);
}

void
Client::get_login(Callback<mtx::responses::LoginFlows> cb)
{
    get<mtx::responses::LoginFlows>(
      "/client/v3/login",
      [cb = std::move(cb)](const mtx::responses::LoginFlows &res, HeaderFields, RequestErr err) {
          cb(res, err);
      },
      false);
}

std::string
Client::login_sso_redirect(std::string redirectUrl, const std::string &idp)
{
    const std::string idp_suffix = idp.empty() ? idp : ("/" + mtx::client::utils::url_encode(idp));
    return protocol_ + "://" + server() + ":" + std::to_string(port()) +
           "/_matrix/client/v3/login/sso/redirect" + idp_suffix + "?" +
           mtx::client::utils::query_params({{"redirectUrl", redirectUrl}});
}

void
Client::well_known(Callback<mtx::responses::WellKnown> callback)
{
    get<mtx::responses::WellKnown>(
      "/matrix/client",
      [cb = std::move(callback)](
        const mtx::responses::WellKnown &res, HeaderFields, RequestErr err) { cb(res, err); },
      false,
      "/.well-known",
      30);
}

void
Client::logout(Callback<mtx::responses::Logout> callback)
{
    mtx::requests::Logout req;

    post<mtx::requests::Logout, mtx::responses::Logout>(
      "/client/v3/logout",
      req,
      [_this    = shared_from_this(),
       callback = std::move(callback)](const mtx::responses::Logout &res, RequestErr err) {
          if (!err) {
              // Clear the now invalid access token when logout is successful
              _this->clear();
          }
          // Pass up response and error to supplied callback
          callback(res, err);
      });
}

void
Client::notifications(uint64_t limit,
                      const std::string &from,
                      const std::string &only,
                      Callback<mtx::responses::Notifications> cb)
{
    std::map<std::string, std::string> params;
    params.emplace("limit", std::to_string(limit));

    if (!from.empty()) {
        params.emplace("from", from);
    }

    if (!only.empty()) {
        params.emplace("only", only);
    }

    get<mtx::responses::Notifications>(
      "/client/v3/notifications?" + mtx::client::utils::query_params(params),
      [cb = std::move(cb)](const mtx::responses::Notifications &res, HeaderFields, RequestErr err) {
          cb(res, err);
      });
}

void
Client::get_pushrules(Callback<mtx::pushrules::GlobalRuleset> cb)
{
    get<mtx::pushrules::GlobalRuleset>(
      "/client/v3/pushrules/",
      [cb = std::move(cb)](const mtx::pushrules::GlobalRuleset &res, HeaderFields, RequestErr err) {
          cb(res, err);
      });
}

void
Client::get_pushrules(const std::string &scope,
                      const std::string &kind,
                      const std::string &ruleId,
                      Callback<mtx::pushrules::PushRule> cb)
{
    get<mtx::pushrules::PushRule>(
      "/client/v3/pushrules/" + mtx::client::utils::url_encode(scope) + "/" +
        mtx::client::utils::url_encode(kind) + "/" + mtx::client::utils::url_encode(ruleId),
      [cb = std::move(cb)](const mtx::pushrules::PushRule &res, HeaderFields, RequestErr err) {
          cb(res, err);
      });
}

void
Client::delete_pushrules(const std::string &scope,
                         const std::string &kind,
                         const std::string &ruleId,
                         ErrCallback cb)
{
    delete_("/client/v3/pushrules/" + mtx::client::utils::url_encode(scope) + "/" +
              mtx::client::utils::url_encode(kind) + "/" + mtx::client::utils::url_encode(ruleId),
            std::move(cb));
}

void
Client::put_pushrules(const std::string &scope,
                      const std::string &kind,
                      const std::string &ruleId,
                      const mtx::pushrules::PushRule &rule,
                      ErrCallback cb,
                      const std::string &before,
                      const std::string &after)
{
    std::map<std::string, std::string> params;

    if (!before.empty())
        params.emplace("before", before);

    if (!after.empty())
        params.emplace("after", after);

    std::string path = "/client/v3/pushrules/" + mtx::client::utils::url_encode(scope) + "/" +
                       mtx::client::utils::url_encode(kind) + "/" +
                       mtx::client::utils::url_encode(ruleId);
    if (!params.empty())
        path += "?" + mtx::client::utils::query_params(params);
    put<mtx::pushrules::PushRule>(path, rule, std::move(cb));
}

void
Client::get_pushrules_enabled(const std::string &scope,
                              const std::string &kind,
                              const std::string &ruleId,
                              Callback<mtx::pushrules::Enabled> cb)
{
    get<mtx::pushrules::Enabled>("/client/v3/pushrules/" + mtx::client::utils::url_encode(scope) +
                                   "/" + mtx::client::utils::url_encode(kind) + "/" +
                                   mtx::client::utils::url_encode(ruleId) + "/enabled",
                                 [cb = std::move(cb)](const mtx::pushrules::Enabled &res,
                                                      HeaderFields,
                                                      RequestErr err) { cb(res, err); });
}

void
Client::put_pushrules_enabled(const std::string &scope,
                              const std::string &kind,
                              const std::string &ruleId,
                              bool enabled,
                              ErrCallback cb)
{
    put<mtx::pushrules::Enabled>("/client/v3/pushrules/" + mtx::client::utils::url_encode(scope) +
                                   "/" + mtx::client::utils::url_encode(kind) + "/" +
                                   mtx::client::utils::url_encode(ruleId) + "/enabled",
                                 {enabled},
                                 std::move(cb));
}

void
Client::get_pushrules_actions(const std::string &scope,
                              const std::string &kind,
                              const std::string &ruleId,
                              Callback<mtx::pushrules::actions::Actions> cb)
{
    get<mtx::pushrules::actions::Actions>(
      "/client/v3/pushrules/" + mtx::client::utils::url_encode(scope) + "/" +
        mtx::client::utils::url_encode(kind) + "/" + mtx::client::utils::url_encode(ruleId) +
        "/actions",
      [cb = std::move(cb)](const mtx::pushrules::actions::Actions &res,
                           HeaderFields,
                           RequestErr err) { cb(res, err); });
}

void
Client::put_pushrules_actions(const std::string &scope,
                              const std::string &kind,
                              const std::string &ruleId,
                              const mtx::pushrules::actions::Actions &actions,
                              ErrCallback cb)
{
    put<mtx::pushrules::actions::Actions>("/client/v3/pushrules/" +
                                            mtx::client::utils::url_encode(scope) + "/" +
                                            mtx::client::utils::url_encode(kind) + "/" +
                                            mtx::client::utils::url_encode(ruleId) + "/actions",
                                          actions,
                                          std::move(cb));
}

void
Client::set_avatar_url(const std::string &avatar_url, ErrCallback callback)
{
    mtx::requests::AvatarUrl req;
    req.avatar_url = avatar_url;

    put<mtx::requests::AvatarUrl>(
      "/client/v3/profile/" + mtx::client::utils::url_encode(user_id_.to_string()) + "/avatar_url",
      req,
      std::move(callback));
}

void
Client::set_displayname(const std::string &displayname, ErrCallback callback)
{
    mtx::requests::DisplayName req;
    req.displayname = displayname;

    put<mtx::requests::DisplayName>(
      "/client/v3/profile/" + mtx::client::utils::url_encode(user_id_.to_string()) + "/displayname",
      req,
      std::move(callback));
}

void
Client::get_profile(const std::string &user_id, Callback<mtx::responses::Profile> callback)
{
    get<mtx::responses::Profile>(
      "/client/v3/profile/" + mtx::client::utils::url_encode(user_id),
      [callback = std::move(callback)](
        const mtx::responses::Profile &res, HeaderFields, RequestErr err) { callback(res, err); });
}

void
Client::get_avatar_url(const std::string &user_id, Callback<mtx::responses::AvatarUrl> callback)
{
    get<mtx::responses::AvatarUrl>(
      "/client/v3/profile/" + mtx::client::utils::url_encode(user_id) + "/avatar_url",
      [callback = std::move(callback)](const mtx::responses::AvatarUrl &res,
                                       HeaderFields,
                                       RequestErr err) { callback(res, err); });
}

void
Client::get_tags(const std::string &room_id, Callback<mtx::events::account_data::Tags> cb)
{
    get<mtx::events::account_data::Tags>(
      "/client/v3/user/" + mtx::client::utils::url_encode(user_id_.to_string()) + "/rooms/" +
        mtx::client::utils::url_encode(room_id) + "/tags",
      [cb = std::move(cb)](const mtx::events::account_data::Tags &res,
                           HeaderFields,
                           RequestErr err) { cb(res, err); });
}
void
Client::put_tag(const std::string &room_id,
                const std::string &tag_name,
                const mtx::events::account_data::Tag &tag,
                ErrCallback cb)
{
    put<mtx::events::account_data::Tag>("/client/v3/user/" +
                                          mtx::client::utils::url_encode(user_id_.to_string()) +
                                          "/rooms/" + mtx::client::utils::url_encode(room_id) +
                                          "/tags/" + mtx::client::utils::url_encode(tag_name),
                                        tag,
                                        std::move(cb));
}
void
Client::delete_tag(const std::string &room_id, const std::string &tag_name, ErrCallback cb)
{
    delete_("/client/v3/user/" + mtx::client::utils::url_encode(user_id_.to_string()) + "/rooms/" +
              mtx::client::utils::url_encode(room_id) + "/tags/" +
              mtx::client::utils::url_encode(tag_name),
            std::move(cb));
}

void
Client::create_room(const mtx::requests::CreateRoom &room_options,
                    Callback<mtx::responses::CreateRoom> callback)
{
    post<mtx::requests::CreateRoom, mtx::responses::CreateRoom>(
      "/client/v3/createRoom", room_options, std::move(callback));
}

void
Client::join_room(const std::string &room, Callback<mtx::responses::RoomId> callback)
{
    join_room(room, {}, std::move(callback));
}

void
Client::join_room(const std::string &room,
                  const std::vector<std::string> &via,
                  Callback<mtx::responses::RoomId> callback,
                  const std::string &reason)
{
    using mtx::client::utils::url_encode;
    std::string query;
    if (!via.empty()) {
        query = "?server_name=" + url_encode(via[0]);
        for (size_t i = 1; i < via.size(); i++) {
            query += "&server_name=" + url_encode(via[i]);
        }
    }
    auto api_path = "/client/v3/join/" + url_encode(room) + query;

    auto body = nlohmann::json::object();
    if (!reason.empty())
        body["reason"] = reason;

    post<std::string, mtx::responses::RoomId>(api_path, body.dump(), std::move(callback));
}

void
Client::knock_room(const std::string &room,
                   const std::vector<std::string> &via,
                   Callback<mtx::responses::RoomId> cb,
                   const std::string &reason)
{
    using mtx::client::utils::url_encode;
    std::string query;
    if (!via.empty()) {
        query = "?server_name=" + url_encode(via[0]);
        for (size_t i = 1; i < via.size(); i++) {
            query += "&server_name=" + url_encode(via[i]);
        }
    }
    auto api_path = "/client/v3/knock/" + url_encode(room) + query;

    auto body = nlohmann::json::object();
    if (!reason.empty())
        body["reason"] = reason;

    post<std::string, mtx::responses::RoomId>(api_path, body.dump(), std::move(cb));
}

void
Client::leave_room(const std::string &room_id,
                   Callback<mtx::responses::Empty> callback,
                   const std::string &reason)
{
    auto api_path = "/client/v3/rooms/" + mtx::client::utils::url_encode(room_id) + "/leave";

    auto body = nlohmann::json::object();
    if (!reason.empty())
        body["reason"] = reason;

    post<std::string, mtx::responses::Empty>(api_path, body.dump(), std::move(callback));
}

void
Client::invite_user(const std::string &room_id,
                    const std::string &user_id,
                    Callback<mtx::responses::RoomInvite> callback,
                    const std::string &reason)
{
    mtx::requests::RoomMembershipChange req;
    req.user_id = user_id;
    req.reason  = reason;

    auto api_path = "/client/v3/rooms/" + mtx::client::utils::url_encode(room_id) + "/invite";

    post<mtx::requests::RoomMembershipChange, mtx::responses::RoomInvite>(
      api_path, req, std::move(callback));
}

void
Client::kick_user(const std::string &room_id,
                  const std::string &user_id,
                  Callback<mtx::responses::Empty> callback,
                  const std::string &reason)
{
    mtx::requests::RoomMembershipChange req;
    req.user_id = user_id;
    req.reason  = reason;

    auto api_path = "/client/v3/rooms/" + mtx::client::utils::url_encode(room_id) + "/kick";

    post<mtx::requests::RoomMembershipChange, mtx::responses::Empty>(
      api_path, req, std::move(callback));
}

void
Client::ban_user(const std::string &room_id,
                 const std::string &user_id,
                 Callback<mtx::responses::Empty> callback,
                 const std::string &reason)
{
    mtx::requests::RoomMembershipChange req;
    req.user_id = user_id;
    req.reason  = reason;

    auto api_path = "/client/v3/rooms/" + mtx::client::utils::url_encode(room_id) + "/ban";

    post<mtx::requests::RoomMembershipChange, mtx::responses::Empty>(
      api_path, req, std::move(callback));
}

void
Client::unban_user(const std::string &room_id,
                   const std::string &user_id,
                   Callback<mtx::responses::Empty> callback,
                   const std::string &reason)
{
    mtx::requests::RoomMembershipChange req;
    req.user_id = user_id;
    req.reason  = reason;

    auto api_path = "/client/v3/rooms/" + mtx::client::utils::url_encode(room_id) + "/unban";

    post<mtx::requests::RoomMembershipChange, mtx::responses::Empty>(
      api_path, req, std::move(callback));
}

void
Client::sync(const SyncOpts &opts, Callback<mtx::responses::Sync> callback)
{
    std::map<std::string, std::string> params;

    if (!opts.filter.empty())
        params.emplace("filter", opts.filter);

    if (!opts.since.empty())
        params.emplace("since", opts.since);

    if (opts.full_state)
        params.emplace("full_state", "true");

    if (opts.set_presence)
        params.emplace("set_presence", mtx::presence::to_string(opts.set_presence.value()));

    params.emplace("timeout", std::to_string(opts.timeout));

    get<mtx::responses::Sync>(
      "/client/v3/sync?" + mtx::client::utils::query_params(params),
      [callback = std::move(callback)](
        const mtx::responses::Sync &res, HeaderFields, RequestErr err) { callback(res, err); });
}

void
Client::versions(Callback<mtx::responses::Versions> callback)
{
    get<mtx::responses::Versions>(
      "/client/versions",
      [callback = std::move(callback)](
        const mtx::responses::Versions &res, HeaderFields, RequestErr err) { callback(res, err); });
}

void
Client::capabilities(Callback<mtx::responses::capabilities::Capabilities> callback)
{
    get<mtx::responses::capabilities::Capabilities>(
      "/client/v3/capabilities",
      [callback = std::move(callback)](const mtx::responses::capabilities::Capabilities &res,
                                       HeaderFields,
                                       RequestErr err) { callback(res, err); });
}

void
Client::upload(const std::string &data,
               const std::string &content_type,
               const std::string &filename,
               Callback<mtx::responses::ContentURI> cb)
{
    std::map<std::string, std::string> params = {{"filename", filename}};

    const auto api_path = "/media/v3/upload?" + client::utils::query_params(params);
    post<std::string, mtx::responses::ContentURI>(
      api_path, data, std::move(cb), true, content_type);
}

std::string
mtx::http::Client::mxc_to_download_url(const std::string &mxc_url)
{
    auto url = mtx::client::utils::parse_mxc_url(mxc_url);
    return endpoint_to_url("/media/v3/download/" + url.server + "/" + url.media_id);
}

void
Client::download(const std::string &mxc_url,
                 std::function<void(const std::string &res,
                                    const std::string &content_type,
                                    const std::string &original_filename,
                                    RequestErr err)> callback)
{
    auto url = mtx::client::utils::parse_mxc_url(mxc_url);
    download(url.server, url.media_id, std::move(callback));
}

void
Client::get_thumbnail(const ThumbOpts &opts, Callback<std::string> callback, bool try_download)
{
    std::map<std::string, std::string> params;
    params.emplace("width", std::to_string(opts.width));
    params.emplace("height", std::to_string(opts.height));
    params.emplace("method", opts.method);

    auto mxc            = mtx::client::utils::parse_mxc_url(opts.mxc_url);
    const auto api_path = "/media/v3/thumbnail/" + mxc.server + "/" + mxc.media_id + "?" +
                          client::utils::query_params(params);
    get<std::string>(
      api_path,
      [callback = std::move(callback),
       try_download,
       mxc   = std::move(mxc),
       _this = shared_from_this()](const std::string &res, HeaderFields, RequestErr err) {
          if (err && try_download) {
              const int status_code = static_cast<int>(err->status_code);

              if (status_code == 404) {
                  _this->download(mxc.server,
                                  mxc.media_id,
                                  [callback](const std::string &res,
                                             const std::string &, // content_type
                                             const std::string &, // original_filename
                                             RequestErr err) { callback(res, err); });
                  return;
              }
          }

          callback(res, err);
      });
}

void
Client::download(const std::string &server,
                 const std::string &media_id,
                 std::function<void(const std::string &res,
                                    const std::string &content_type,
                                    const std::string &original_filename,
                                    RequestErr err)> callback)
{
    const auto api_path = "/media/v3/download/" + server + "/" + media_id;
    get<std::string>(
      api_path,
      [callback =
         std::move(callback)](const std::string &res, HeaderFields fields, RequestErr err) {
          std::string content_type, original_filename;

          if (fields) {
              if (fields->find("Content-Type") != fields->end())
                  content_type = fields->at("Content-Type");
              if (fields->find("Content-Disposition") != fields->end()) {
                  auto value = fields->at("Content-Disposition");

                  if (auto pos = value.find("filename"); pos != std::string::npos) {
                      if (auto start = value.find('"', pos); start != std::string::npos) {
                          auto end          = value.find('"', start + 1);
                          original_filename = value.substr(start + 1, end - start - 2);
                      } else if (start = value.find('='); start != std::string::npos) {
                          original_filename = value.substr(start + 1);
                      }
                  }
              }
          }

          callback(res, content_type, original_filename, err);
      });
}

void
Client::start_typing(const std::string &room_id, uint64_t timeout, ErrCallback callback)
{
    using mtx::client::utils::url_encode;
    const auto api_path =
      "/client/v3/rooms/" + url_encode(room_id) + "/typing/" + url_encode(user_id_.to_string());

    mtx::requests::TypingNotification req;
    req.typing  = true;
    req.timeout = timeout;

    put<mtx::requests::TypingNotification>(api_path, req, std::move(callback));
}

void
Client::stop_typing(const std::string &room_id, ErrCallback callback)
{
    using mtx::client::utils::url_encode;
    const auto api_path =
      "/client/v3/rooms/" + url_encode(room_id) + "/typing/" + url_encode(user_id_.to_string());

    mtx::requests::TypingNotification req;
    req.typing = false;

    put<mtx::requests::TypingNotification>(api_path, req, std::move(callback));
}

void
Client::presence_status(const std::string &user_id,
                        Callback<mtx::events::presence::Presence> callback)
{
    using mtx::client::utils::url_encode;
    const auto api_path = "/client/v3/presence/" + url_encode(user_id) + "/status";
    get<mtx::events::presence::Presence>(
      api_path,
      [callback = std::move(callback)](const mtx::events::presence::Presence &res,
                                       HeaderFields,
                                       RequestErr err) { callback(res, err); });
}
void
Client::put_presence_status(mtx::presence::PresenceState state,
                            std::optional<std::string> status_msg,
                            ErrCallback cb)
{
    using mtx::client::utils::url_encode;
    const auto api_path = "/client/v3/presence/" + url_encode(user_id_.to_string()) + "/status";

    nlohmann::json body;
    body["presence"] = mtx::presence::to_string(state);
    if (status_msg)
        body["status_msg"] = *status_msg;

    put<nlohmann::json>(api_path, body, std::move(cb));
}

void
Client::get_event(const std::string &room_id,
                  const std::string &event_id,
                  Callback<mtx::events::collections::TimelineEvents> callback)
{
    using namespace mtx::client::utils;
    const auto api_path =
      "/client/v3/rooms/" + url_encode(room_id) + "/event/" + url_encode(event_id);

    get<mtx::events::collections::TimelineEvent>(
      api_path,
      [callback = std::move(callback)](const mtx::events::collections::TimelineEvent &res,
                                       HeaderFields,
                                       RequestErr err) { callback(res.data, err); });
}

void
Client::members(const std::string &room_id,
                Callback<mtx::responses::Members> cb,
                const std::string &at,
                std::optional<mtx::events::state::Membership> membership,
                std::optional<mtx::events::state::Membership> not_membership)
{
    std::map<std::string, std::string> params;

    std::string query;

    if (!at.empty())
        params.emplace("at", at);
    if (membership)
        params.emplace("membership", events::state::membershipToString(*membership));
    if (not_membership)
        params.emplace("not_membership", events::state::membershipToString(*not_membership));

    const auto api_path = "/client/v3/rooms/" + mtx::client::utils::url_encode(room_id) +
                          "/members?" + client::utils::query_params(params);

    get<mtx::responses::Members>(api_path,
                                 [cb = std::move(cb)](const mtx::responses::Members &res,
                                                      HeaderFields,
                                                      RequestErr err) { cb(res, err); });
}

void
Client::messages(const MessagesOpts &opts, Callback<mtx::responses::Messages> callback)
{
    std::map<std::string, std::string> params;

    params.emplace("dir", to_string(opts.dir));

    if (!opts.from.empty())
        params.emplace("from", opts.from);
    if (!opts.to.empty())
        params.emplace("to", opts.to);
    if (opts.limit > 0)
        params.emplace("limit", std::to_string(opts.limit));
    if (!opts.filter.empty())
        params.emplace("filter", opts.filter);

    const auto api_path = "/client/v3/rooms/" + mtx::client::utils::url_encode(opts.room_id) +
                          "/messages?" + client::utils::query_params(params);

    get<mtx::responses::Messages>(
      api_path,
      [callback = std::move(callback)](
        const mtx::responses::Messages &res, HeaderFields, RequestErr err) { callback(res, err); });
}

void
Client::upload_filter(const nlohmann::json &j, Callback<mtx::responses::FilterId> callback)
{
    const auto api_path =
      "/client/v3/user/" + mtx::client::utils::url_encode(user_id_.to_string()) + "/filter";

    post<nlohmann::json, mtx::responses::FilterId>(api_path, j, std::move(callback));
}

void
Client::read_event(const std::string &room_id,
                   const std::string &event_id,
                   ErrCallback callback,
                   bool hidden)
{
    const auto api_path =
      "/client/v3/rooms/" + mtx::client::utils::url_encode(room_id) + "/read_markers";

    nlohmann::json body = {
      {"m.fully_read", event_id},
      {"org.matrix.msc2285.read.private", event_id},
      {"m.read.private", event_id},
    };

    if (!hidden)
        body["m.read"] = event_id;

    post<nlohmann::json, mtx::responses::Empty>(
      api_path,
      body,
      [callback = std::move(callback)](const mtx::responses::Empty, RequestErr err) {
          callback(err);
      });
}

void
Client::redact_event(const std::string &room_id,
                     const std::string &event_id,
                     Callback<mtx::responses::EventId> callback,
                     const std::string &reason)
{
    const auto api_path = "/client/v3/rooms/" + mtx::client::utils::url_encode(room_id) +
                          "/redact/" + mtx::client::utils::url_encode(event_id) + "/" +
                          mtx::client::utils::url_encode(mtx::client::utils::random_token());

    nlohmann::json body = nlohmann::json::object();
    if (!reason.empty()) {
        body["reason"] = reason;
    }

    put<nlohmann::json, mtx::responses::EventId>(api_path, body, std::move(callback));
}

void
Client::registration(const std::string &user,
                     const std::string &pass,
                     UIAHandler uia_handler,
                     Callback<mtx::responses::Register> cb,
                     const std::string &initial_device_display_name)
{
    nlohmann::json req = {{"username", user}, {"password", pass}};

    if (!initial_device_display_name.empty())
        req["initial_device_display_name"] = initial_device_display_name;

    uia_handler.next_ = [this, req, cb = std::move(cb)](const UIAHandler &h,
                                                        const nlohmann::json &auth) {
        auto request = req;
        if (!auth.empty())
            request["auth"] = auth;

        post<nlohmann::json, mtx::responses::Register>(
          "/client/v3/register",
          request,
          [this, cb, h](auto &r, RequestErr e) {
              if (e && e->status_code == 401) {
                  mtx::utils::log::log()->debug("{}", e);
                  h.prompt(h, e->matrix_error.unauthorized);
              } else {
                  if (!e && !r.access_token.empty()) {
                      this->user_id_      = r.user_id;
                      this->device_id_    = r.device_id;
                      this->access_token_ = r.access_token;
                  }
                  cb(r, e);
              }
          },
          false);
    };

    uia_handler.next_(uia_handler, {});
}

void
Client::registration(Callback<mtx::responses::Register> cb)
{
    post<nlohmann::json, mtx::responses::Register>(
      "/client/v3/register", nlohmann::json::object(), std::move(cb), false);
}

void
Client::registration_token_validity(const std::string token,
                                    Callback<mtx::responses::RegistrationTokenValidity> cb)
{
    const auto api_path = "/client/v1/register/m.login.registration_token/validity?" +
                          mtx::client::utils::query_params({{"token", token}});

    get<mtx::responses::RegistrationTokenValidity>(
      api_path,
      [cb = std::move(cb)](const mtx::responses::RegistrationTokenValidity &res,
                           HeaderFields,
                           RequestErr err) { cb(res, err); });
}

void
Client::register_email_request_token(const requests::RequestEmailToken &r,
                                     Callback<mtx::responses::RequestToken> cb)
{
    post("/client/v3/register/email/requestToken", r, std::move(cb));
}

void
Client::register_username_available(const std::string &username,
                                    Callback<mtx::responses::Available> cb)
{
    get<mtx::responses::Available>(
      "/client/v3/register/available?username=" + mtx::client::utils::url_encode(username),
      [cb = std::move(cb)](const mtx::responses::Available &res, HeaderFields, RequestErr err) {
          cb(res, err);
      });
}

void
Client::verify_email_request_token(const requests::RequestEmailToken &r,
                                   Callback<mtx::responses::RequestToken> cb)
{
    post("/client/v3/account/password/email/requestToken", r, std::move(cb));
}

void
Client::register_phone_request_token(const requests::RequestMSISDNToken &r,
                                     Callback<mtx::responses::RequestToken> cb)
{
    post("/client/v3/register/msisdn/requestToken", r, std::move(cb));
}
void
Client::verify_phone_request_token(const requests::RequestMSISDNToken &r,
                                   Callback<mtx::responses::RequestToken> cb)
{
    post("/client/v3/account/password/msisdn/requestToken", r, std::move(cb));
}

void
Client::validate_submit_token(const std::string &url,
                              const requests::IdentitySubmitToken &r,
                              Callback<mtx::responses::Success> cb)
{
    // some dancing to send to an arbitrary, server provided url
    auto callback = prepare_callback<mtx::responses::Success>(
      [cb = std::move(cb)](const mtx::responses::Success &res, HeaderFields, RequestErr err) {
          cb(res, err);
      });
    p->client.post(
      url,
      nlohmann::json(r).dump(),
      "application/json",
      [callback = std::move(callback)](const coeurl::Request &r) {
          callback(r.response_headers(), r.response(), r.error_code(), r.response_code());
      },
      prepare_headers(false));
}

void
Client::send_state_event(const std::string &room_id,
                         const std::string &event_type,
                         const std::string &state_key,
                         const nlohmann::json &payload,
                         Callback<mtx::responses::EventId> callback)
{
    const auto api_path = "/client/v3/rooms/" + mtx::client::utils::url_encode(room_id) +
                          "/state/" + mtx::client::utils::url_encode(event_type) + "/" +
                          mtx::client::utils::url_encode(state_key);

    put<nlohmann::json, mtx::responses::EventId>(api_path, payload, std::move(callback));
}

void
Client::send_to_device(const std::string &event_type,
                       const std::string &txn_id,
                       const nlohmann::json &body,
                       ErrCallback callback)
{
    const auto api_path = "/client/v3/sendToDevice/" + mtx::client::utils::url_encode(event_type) +
                          "/" + mtx::client::utils::url_encode(txn_id);

    put<nlohmann::json>(api_path, body, std::move(callback));
}

void
Client::resolve_room_alias(const std::string &alias, Callback<mtx::responses::RoomId> cb)
{
    const auto api_path = "/client/v3/directory/room/" + mtx::client::utils::url_encode(alias);

    get<mtx::responses::RoomId>(api_path,
                                [cb = std::move(cb)](const mtx::responses::RoomId &res,
                                                     HeaderFields,
                                                     RequestErr err) { cb(res, err); });
}
void
Client::add_room_alias(const std::string &alias, const std::string &roomid, ErrCallback cb)
{
    const auto api_path = "/client/v3/directory/room/" + mtx::client::utils::url_encode(alias);
    auto body           = nlohmann::json::object();
    body["room_id"]     = roomid;
    put<nlohmann::json>(api_path, body, std::move(cb));
}

void
Client::delete_room_alias(const std::string &alias, ErrCallback cb)
{
    delete_("/client/v3/directory/room/" + mtx::client::utils::url_encode(alias), std::move(cb));
}

void
Client::list_room_aliases(const std::string &room_id, Callback<mtx::responses::Aliases> cb)
{
    const auto api_path =
      "/client/v3/rooms/" + mtx::client::utils::url_encode(room_id) + "/aliases";

    get<mtx::responses::Aliases>(api_path,
                                 [cb = std::move(cb)](const mtx::responses::Aliases &res,
                                                      HeaderFields,
                                                      RequestErr err) { cb(res, err); });
}

void
Client::get_room_visibility(const std::string &room_id,
                            Callback<mtx::responses::PublicRoomVisibility> cb)
{
    const auto api_path =
      "/client/v3/directory/list/room/" + mtx::client::utils::url_encode(room_id);

    get<mtx::responses::PublicRoomVisibility>(
      api_path,
      [cb = std::move(cb)](const mtx::responses::PublicRoomVisibility &res,
                           HeaderFields,
                           RequestErr err) { cb(res, err); });
}

void
Client::put_room_visibility(const std::string &room_id,
                            const mtx::requests::PublicRoomVisibility &req,
                            ErrCallback cb)
{
    const auto api_path =
      "/client/v3/directory/list/room/" + mtx::client::utils::url_encode(room_id);
    put<mtx::requests::PublicRoomVisibility>(api_path, req, std::move(cb));
}

void
Client::post_public_rooms(const mtx::requests::PublicRooms &req,
                          Callback<mtx::responses::PublicRooms> cb,
                          const std::string &server)
{
    std::string api_path = "/client/v3/publicRooms";

    if (!server.empty())
        api_path += "?" + mtx::client::utils::query_params({{"server", server}});
    post<mtx::requests::PublicRooms, mtx::responses::PublicRooms>(api_path, req, std::move(cb));
}

void
Client::get_public_rooms(Callback<mtx::responses::PublicRooms> cb,
                         const std::string &server,
                         size_t limit,
                         const std::string &since)
{
    std::string api_path = "/client/v3/publicRooms";

    std::map<std::string, std::string> params;
    if (!server.empty())
        params["server"] = server;
    if (limit > 0)
        params["limit"] = std::to_string(limit);
    if (!since.empty())
        params["since"] = since;

    if (!params.empty())
        api_path += "?" + mtx::client::utils::query_params(params);

    get<mtx::responses::PublicRooms>(api_path,
                                     [cb = std::move(cb)](const mtx::responses::PublicRooms &res,
                                                          HeaderFields,
                                                          RequestErr err) { cb(res, err); });
}

void
Client::get_hierarchy(const std::string &room_id,
                      Callback<mtx::responses::HierarchyRooms> cb,
                      const std::string &from,
                      size_t limit,
                      size_t max_depth,
                      bool suggested_only)
{
    std::string api_path =
      "/client/v1/rooms/" + mtx::client::utils::url_encode(room_id) + "/hierarchy";

    std::map<std::string, std::string> params;
    if (limit > 0)
        params["limit"] = std::to_string(limit);
    if (max_depth > 0)
        params["max_depth"] = std::to_string(max_depth);
    if (suggested_only)
        params["suggested_only"] = "true";
    if (!from.empty())
        params["from"] = from;

    if (!params.empty())
        api_path += "?" + mtx::client::utils::query_params(params);

    get<mtx::responses::HierarchyRooms>(
      api_path,
      [cb = std::move(cb)](
        const mtx::responses::HierarchyRooms &res, HeaderFields, RequestErr err) { cb(res, err); });
}

void
Client::get_summary(const std::string &room_id,
                    Callback<mtx::responses::PublicRoomsChunk> cb,
                    std::vector<std::string> via)
{
    std::string query;
    if (!via.empty()) {
        query = "?via=" + mtx::client::utils::url_encode(via[0]);
        for (size_t i = 1; i < via.size(); i++) {
            query += "&via=" + mtx::client::utils::url_encode(via[i]);
        }
    }
    std::string api_path = "/client/unstable/im.nheko.summary/rooms/" +
                           mtx::client::utils::url_encode(room_id) + "/summary" + query;

    get<mtx::responses::PublicRoomsChunk>(
      api_path,
      [this, room_id, cb = std::move(cb)](
        const mtx::responses::PublicRoomsChunk &res, HeaderFields, RequestErr err) {
          if (!err || !(err->status_code == 404 || err->status_code == 400))
              cb(res, err);
          else if (!room_id.empty() && room_id[0] == '#')
              resolve_room_alias(
                room_id, [this, cb](const mtx::responses::RoomId &room, RequestErr err) {
                    if (room.room_id.empty())
                        cb({}, err);
                    else
                        get_hierarchy(
                          room.room_id,
                          [cb](const mtx::responses::HierarchyRooms &res, RequestErr err) {
                              if (res.rooms.empty())
                                  cb({}, err);
                              else
                                  cb(res.rooms.front(), err);
                          },
                          "",
                          1);
                });
          else
              get_hierarchy(
                room_id,
                [cb](const mtx::responses::HierarchyRooms &res, RequestErr err) {
                    if (res.rooms.empty())
                        cb({}, err);
                    else
                        cb(res.rooms.front(), err);
                },
                "",
                1);
      });
}

//
// Device related endpoints
//

void
Client::query_devices(Callback<mtx::responses::QueryDevices> cb)
{
    get<mtx::responses::QueryDevices>("/client/v3/devices",
                                      [cb = std::move(cb)](const mtx::responses::QueryDevices &res,
                                                           HeaderFields,
                                                           RequestErr err) { cb(res, err); });
}

void
Client::get_device(const std::string &device_id, Callback<mtx::responses::Device> cb)
{
    get<mtx::responses::Device>("/client/v3/devices/" + mtx::client::utils::url_encode(device_id),
                                [cb = std::move(cb)](const mtx::responses::Device &res,
                                                     HeaderFields,
                                                     RequestErr err) { cb(res, err); });
}

void
Client::set_device_name(const std::string &device_id,
                        const std::string &display_name,
                        ErrCallback callback)
{
    mtx::requests::DeviceUpdate req;
    req.display_name = display_name;

    put<mtx::requests::DeviceUpdate>(
      "/client/v3/devices/" + mtx::client::utils::url_encode(device_id), req, std::move(callback));
}

void
Client::delete_device(const std::string &device_id, UIAHandler uia_handler, ErrCallback cb)
{
    nlohmann::json req;
    req["devices"] = {device_id};

    uia_handler.next_ = [this, req, cb = std::move(cb)](const UIAHandler &h,
                                                        const nlohmann::json &auth) {
        auto request = req;
        if (!auth.empty())
            request["auth"] = auth;

        post<nlohmann::json, mtx::responses::Empty>(
          "/client/v3/delete_devices", request, [cb, h](auto &, RequestErr e) {
              if (e && e->status_code == 401 && !e->matrix_error.unauthorized.flows.empty())
                  h.prompt(h, e->matrix_error.unauthorized);
              else
                  cb(e);
          });
    };

    uia_handler.next_(uia_handler, {});
}

void
Client::delete_devices(const std::vector<std::string> &device_ids,
                       UIAHandler uia_handler,
                       ErrCallback cb)
{
    nlohmann::json req;
    req["devices"] = device_ids;

    uia_handler.next_ = [this, req = std::move(req), cb = std::move(cb)](
                          const UIAHandler &h, const nlohmann::json &auth) {
        auto request = req;
        if (!auth.empty())
            request["auth"] = auth;

        post<nlohmann::json, mtx::responses::Empty>(
          "/client/v3/delete_devices", request, [cb, h](auto &, RequestErr e) {
              if (e && e->status_code == 401 && !e->matrix_error.unauthorized.flows.empty())
                  h.prompt(h, e->matrix_error.unauthorized);
              else
                  cb(e);
          });
    };

    uia_handler.next_(uia_handler, {});
}

//
// Encryption related endpoints
//

void
Client::upload_keys(const mtx::requests::UploadKeys &req,
                    Callback<mtx::responses::UploadKeys> callback)
{
    post<mtx::requests::UploadKeys, mtx::responses::UploadKeys>(
      "/client/v3/keys/upload", req, std::move(callback));
}

void
Client::keys_signatures_upload(const mtx::requests::KeySignaturesUpload &req,
                               Callback<mtx::responses::KeySignaturesUpload> cb)
{
    post<mtx::requests::KeySignaturesUpload, mtx::responses::KeySignaturesUpload>(
      "/client/v3/keys/signatures/upload", req, std::move(cb));
}

void
Client::device_signing_upload(const mtx::requests::DeviceSigningUpload &deviceKeys,
                              UIAHandler uia_handler,
                              ErrCallback cb)
{
    nlohmann::json req = deviceKeys;

    uia_handler.next_ = [this, req = std::move(req), cb = std::move(cb)](
                          const UIAHandler &h, const nlohmann::json &auth) {
        auto request = req;
        if (!auth.empty())
            request["auth"] = auth;

        post<nlohmann::json, mtx::responses::Empty>(
          "/client/v3/keys/device_signing/upload", request, [cb, h](auto &, RequestErr e) {
              if (e && e->status_code == 401 && !e->matrix_error.unauthorized.flows.empty())
                  h.prompt(h, e->matrix_error.unauthorized);
              else
                  cb(e);
          });
    };

    uia_handler.next_(uia_handler, {});
}

void
Client::query_keys(const mtx::requests::QueryKeys &req,
                   Callback<mtx::responses::QueryKeys> callback)
{
    post<mtx::requests::QueryKeys, mtx::responses::QueryKeys>(
      "/client/v3/keys/query", req, std::move(callback));
}

//! Claims one-time keys for use in pre-key messages.
void
Client::claim_keys(const mtx::requests::ClaimKeys &req, Callback<mtx::responses::ClaimKeys> cb)
{
    post<mtx::requests::ClaimKeys, mtx::responses::ClaimKeys>(
      "/client/v3/keys/claim", req, std::move(cb));
}

void
Client::key_changes(const std::string &from,
                    const std::string &to,
                    Callback<mtx::responses::KeyChanges> callback)
{
    std::map<std::string, std::string> params;

    if (!from.empty())
        params.emplace("from", from);

    if (!to.empty())
        params.emplace("to", to);

    get<mtx::responses::KeyChanges>(
      "/client/v3/keys/changes?" + mtx::client::utils::query_params(params),
      [callback = std::move(callback)](const mtx::responses::KeyChanges &res,
                                       HeaderFields,
                                       RequestErr err) { callback(res, err); });
}

//
// Key backup endpoints
//
void
Client::backup_version(Callback<mtx::responses::backup::BackupVersion> cb)
{
    get<mtx::responses::backup::BackupVersion>(
      "/client/v3/room_keys/version",
      [cb = std::move(cb)](const mtx::responses::backup::BackupVersion &res,
                           HeaderFields,
                           RequestErr err) { cb(res, err); });
}
void
Client::backup_version(const std::string &version,
                       Callback<mtx::responses::backup::BackupVersion> cb)
{
    get<mtx::responses::backup::BackupVersion>(
      "/client/v3/room_keys/version/" + mtx::client::utils::url_encode(version),
      [cb = std::move(cb)](const mtx::responses::backup::BackupVersion &res,
                           HeaderFields,
                           RequestErr err) { cb(res, err); });
}

void
Client::update_backup_version(const std::string &version,
                              const mtx::responses::backup::BackupVersion &data,
                              ErrCallback cb)
{
    put<mtx::responses::backup::BackupVersion>("/client/v3/room_keys/version/" +
                                                 mtx::client::utils::url_encode(version),
                                               data,
                                               std::move(cb));
}

void
Client::post_backup_version(const std::string &algorithm,
                            const std::string &auth_data,
                            Callback<mtx::responses::Version> cb)
{
    nlohmann::json req = {{"algorithm", algorithm},
                          {"auth_data", nlohmann::json::parse(auth_data)}};
    post<nlohmann::json, mtx::responses::Version>(
      "/client/v3/room_keys/version", req, std::move(cb));
}
void
Client::room_keys(const std::string &version, Callback<mtx::responses::backup::KeysBackup> cb)
{
    get<mtx::responses::backup::KeysBackup>(
      "/client/v3/room_keys/keys?" + mtx::client::utils::query_params({{"version", version}}),
      [cb = std::move(cb)](const mtx::responses::backup::KeysBackup &res,
                           HeaderFields,
                           RequestErr err) { cb(res, err); });
}
void
Client::room_keys(const std::string &version,
                  const std::string &room_id,
                  Callback<mtx::responses::backup::RoomKeysBackup> cb)
{
    get<mtx::responses::backup::RoomKeysBackup>(
      "/client/v3/room_keys/keys/" + mtx::client::utils::url_encode(room_id) + "?" +
        mtx::client::utils::query_params({{"version", version}}),
      [cb = std::move(cb)](const mtx::responses::backup::RoomKeysBackup &res,
                           HeaderFields,
                           RequestErr err) { cb(res, err); });
}
void
Client::room_keys(const std::string &version,
                  const std::string &room_id,
                  const std::string &session_id,
                  Callback<mtx::responses::backup::SessionBackup> cb)
{
    get<mtx::responses::backup::SessionBackup>(
      "/client/v3/room_keys/keys/" + mtx::client::utils::url_encode(room_id) + "/" +
        mtx::client::utils::url_encode(session_id) + "?" +
        mtx::client::utils::query_params({{"version", version}}),
      [cb = std::move(cb)](const mtx::responses::backup::SessionBackup &res,
                           HeaderFields,
                           RequestErr err) { cb(res, err); });
}

void
Client::put_room_keys(const std::string &version,
                      const mtx::responses::backup::KeysBackup &keys,
                      ErrCallback cb)
{
    put("/client/v3/room_keys/keys?" + mtx::client::utils::query_params({{"version", version}}),
        keys,
        std::move(cb));
}
void
Client::put_room_keys(const std::string &version,
                      const std::string &room_id,
                      const mtx::responses::backup::RoomKeysBackup &keys,
                      ErrCallback cb)
{
    put("/client/v3/room_keys/keys/" + mtx::client::utils::url_encode(room_id) + "?" +
          mtx::client::utils::query_params({{"version", version}}),
        keys,
        std::move(cb));
}
void
Client::put_room_keys(const std::string &version,
                      const std::string &room_id,
                      const std::string &session_id,
                      const mtx::responses::backup::SessionBackup &keys,
                      ErrCallback cb)
{
    put("/client/v3/room_keys/keys/" + mtx::client::utils::url_encode(room_id) + "/" +
          mtx::client::utils::url_encode(session_id) + "?" +
          mtx::client::utils::query_params({{"version", version}}),
        keys,
        std::move(cb));
}

//! Retrieve a specific secret
void
Client::secret_storage_secret(const std::string &secret_id,
                              Callback<mtx::secret_storage::Secret> cb)
{
    get<mtx::secret_storage::Secret>(
      "/client/v3/user/" + mtx::client::utils::url_encode(user_id_.to_string()) + "/account_data/" +
        mtx::client::utils::url_encode(secret_id),
      [cb = std::move(cb)](const mtx::secret_storage::Secret &res, HeaderFields, RequestErr err) {
          cb(res, err);
      });
}
//! Retrieve information about a key
void
Client::secret_storage_key(const std::string &key_id,
                           Callback<mtx::secret_storage::AesHmacSha2KeyDescription> cb)
{
    get<mtx::secret_storage::AesHmacSha2KeyDescription>(
      "/client/v3/user/" + mtx::client::utils::url_encode(user_id_.to_string()) +
        "/account_data/m.secret_storage.key." + mtx::client::utils::url_encode(key_id),
      [cb = std::move(cb)](const mtx::secret_storage::AesHmacSha2KeyDescription &res,
                           HeaderFields,
                           RequestErr err) { cb(res, err); });
}

//! Upload a specific secret
void
Client::upload_secret_storage_secret(const std::string &secret_id,
                                     const mtx::secret_storage::Secret &secret,
                                     ErrCallback cb)
{
    put("/client/v3/user/" + mtx::client::utils::url_encode(user_id_.to_string()) +
          "/account_data/" + mtx::client::utils::url_encode(secret_id),
        secret,
        std::move(cb));
}

//! Upload information about a key
void
Client::upload_secret_storage_key(const std::string &key_id,
                                  const mtx::secret_storage::AesHmacSha2KeyDescription &desc,
                                  ErrCallback cb)
{
    put("/client/v3/user/" + mtx::client::utils::url_encode(user_id_.to_string()) +
          "/account_data/m.secret_storage.key." + mtx::client::utils::url_encode(key_id),
        desc,
        std::move(cb));
}

void
Client::set_secret_storage_default_key(const std::string &key_id, ErrCallback cb)
{
    nlohmann::json key = {{"key", key_id}};
    put("/client/v3/user/" + mtx::client::utils::url_encode(user_id_.to_string()) +
          "/account_data/m.secret_storage.default_key",
        key,
        std::move(cb));
}

void
Client::enable_encryption(const std::string &room, Callback<mtx::responses::EventId> callback)
{
    using namespace mtx::events;
    state::Encryption event;

    send_state_event<state::Encryption>(room, "", event, std::move(callback));
}

void
Client::get_turn_server(Callback<mtx::responses::TurnServer> cb)
{
    get<mtx::responses::TurnServer>("/client/v3/voip/turnServer",
                                    [cb = std::move(cb)](const mtx::responses::TurnServer &res,
                                                         HeaderFields,
                                                         RequestErr err) { cb(res, err); });
}

void
Client::set_pusher(const mtx::requests::SetPusher &req, Callback<mtx::responses::Empty> cb)
{
    post<mtx::requests::SetPusher, mtx::responses::Empty>(
      "/client/v3/pushers/set", req, std::move(cb));
}

void
Client::search_user_directory(const std::string &search_term,
                              Callback<mtx::responses::Users> callback,
                              int limit)
{
    nlohmann::json req = {{"search_term", search_term}};
    if (limit >= 0)
        req["limit"] = limit;
    post<nlohmann::json, mtx::responses::Users>(
      "/client/v3/user_directory/search", req, std::move(callback));
}

// Template instantiations for the various send functions

#define MTXCLIENT_SEND_STATE_EVENT(Content)                                                        \
    template void mtx::http::Client::send_state_event<mtx::events::Content>(                       \
      const std::string &,                                                                         \
      const std::string &state_key,                                                                \
      const mtx::events::Content &,                                                                \
      Callback<mtx::responses::EventId> cb);                                                       \
    template void mtx::http::Client::send_state_event<mtx::events::Content>(                       \
      const std::string &, const mtx::events::Content &, Callback<mtx::responses::EventId> cb);    \
    template void mtx::http::Client::get_state_event<mtx::events::Content>(                        \
      const std::string &room_id,                                                                  \
      const std::string &type,                                                                     \
      const std::string &state_key,                                                                \
      Callback<mtx::events::Content> cb);                                                          \
    template void mtx::http::Client::get_state_event<mtx::events::Content>(                        \
      const std::string &room_id,                                                                  \
      const std::string &state_key,                                                                \
      Callback<mtx::events::Content> cb);

MTXCLIENT_SEND_STATE_EVENT(state::Aliases)
MTXCLIENT_SEND_STATE_EVENT(state::Avatar)
MTXCLIENT_SEND_STATE_EVENT(state::CanonicalAlias)
MTXCLIENT_SEND_STATE_EVENT(state::Create)
MTXCLIENT_SEND_STATE_EVENT(state::Encryption)
MTXCLIENT_SEND_STATE_EVENT(state::GuestAccess)
MTXCLIENT_SEND_STATE_EVENT(state::HistoryVisibility)
MTXCLIENT_SEND_STATE_EVENT(state::JoinRules)
MTXCLIENT_SEND_STATE_EVENT(state::Member)
MTXCLIENT_SEND_STATE_EVENT(state::Name)
MTXCLIENT_SEND_STATE_EVENT(state::PinnedEvents)
MTXCLIENT_SEND_STATE_EVENT(state::PowerLevels)
MTXCLIENT_SEND_STATE_EVENT(state::Tombstone)
MTXCLIENT_SEND_STATE_EVENT(state::Topic)
MTXCLIENT_SEND_STATE_EVENT(state::Widget)
MTXCLIENT_SEND_STATE_EVENT(state::policy_rule::UserRule)
MTXCLIENT_SEND_STATE_EVENT(state::policy_rule::RoomRule)
MTXCLIENT_SEND_STATE_EVENT(state::policy_rule::ServerRule)
MTXCLIENT_SEND_STATE_EVENT(state::space::Child)
MTXCLIENT_SEND_STATE_EVENT(state::space::Parent)
MTXCLIENT_SEND_STATE_EVENT(msc2545::ImagePack)

#define MTXCLIENT_SEND_ROOM_MESSAGE(Content)                                                       \
    template void mtx::http::Client::send_room_message<Content>(                                   \
      const std::string &,                                                                         \
      const std::string &,                                                                         \
      const Content &,                                                                             \
      Callback<mtx::responses::EventId> cb);                                                       \
    template void mtx::http::Client::send_room_message<Content>(                                   \
      const std::string &, const Content &, Callback<mtx::responses::EventId> cb);

MTXCLIENT_SEND_ROOM_MESSAGE(mtx::events::msg::Encrypted)
MTXCLIENT_SEND_ROOM_MESSAGE(mtx::events::msg::StickerImage)
MTXCLIENT_SEND_ROOM_MESSAGE(mtx::events::msg::Reaction)
MTXCLIENT_SEND_ROOM_MESSAGE(mtx::events::msg::Audio)
MTXCLIENT_SEND_ROOM_MESSAGE(mtx::events::msg::Emote)
MTXCLIENT_SEND_ROOM_MESSAGE(mtx::events::msg::File)
MTXCLIENT_SEND_ROOM_MESSAGE(mtx::events::msg::Image)
MTXCLIENT_SEND_ROOM_MESSAGE(mtx::events::msg::Notice)
MTXCLIENT_SEND_ROOM_MESSAGE(mtx::events::msg::Text)
MTXCLIENT_SEND_ROOM_MESSAGE(mtx::events::msg::Video)
MTXCLIENT_SEND_ROOM_MESSAGE(mtx::events::msg::Confetti)
// MTXCLIENT_SEND_ROOM_MESSAGE(mtx::events::msg::KeyVerificationRequest)
// MTXCLIENT_SEND_ROOM_MESSAGE(mtx::events::msg::KeyVerificationStart)
// MTXCLIENT_SEND_ROOM_MESSAGE(mtx::events::msg::KeyVerificationReady)
// MTXCLIENT_SEND_ROOM_MESSAGE(mtx::events::msg::KeyVerificationDone)
// MTXCLIENT_SEND_ROOM_MESSAGE(mtx::events::msg::KeyVerificationAccept)
// MTXCLIENT_SEND_ROOM_MESSAGE(mtx::events::msg::KeyVerificationCancel)
// MTXCLIENT_SEND_ROOM_MESSAGE(mtx::events::msg::KeyVerificationKey)
// MTXCLIENT_SEND_ROOM_MESSAGE(mtx::events::msg::KeyVerificationMac)
MTXCLIENT_SEND_ROOM_MESSAGE(mtx::events::voip::CallInvite)
MTXCLIENT_SEND_ROOM_MESSAGE(mtx::events::voip::CallCandidates)
MTXCLIENT_SEND_ROOM_MESSAGE(mtx::events::voip::CallAnswer)
MTXCLIENT_SEND_ROOM_MESSAGE(mtx::events::voip::CallHangUp)
MTXCLIENT_SEND_ROOM_MESSAGE(mtx::events::voip::CallSelectAnswer)
MTXCLIENT_SEND_ROOM_MESSAGE(mtx::events::voip::CallReject)
MTXCLIENT_SEND_ROOM_MESSAGE(mtx::events::voip::CallNegotiate)

#define MTXCLIENT_SEND_TO_DEVICE(Content)                                                          \
    template void mtx::http::Client::send_to_device<Content>(                                      \
      const std::string &txid,                                                                     \
      const std::map<mtx::identifiers::User, std::map<std::string, Content>> &messages,            \
      ErrCallback callback);

MTXCLIENT_SEND_TO_DEVICE(mtx::events::msg::RoomKey)
MTXCLIENT_SEND_TO_DEVICE(mtx::events::msg::ForwardedRoomKey)
MTXCLIENT_SEND_TO_DEVICE(mtx::events::msg::KeyRequest)
MTXCLIENT_SEND_TO_DEVICE(mtx::events::msg::OlmEncrypted)
MTXCLIENT_SEND_TO_DEVICE(mtx::events::msg::Encrypted)
MTXCLIENT_SEND_TO_DEVICE(mtx::events::msg::Dummy)
MTXCLIENT_SEND_TO_DEVICE(mtx::events::msg::KeyVerificationRequest)
MTXCLIENT_SEND_TO_DEVICE(mtx::events::msg::KeyVerificationStart)
MTXCLIENT_SEND_TO_DEVICE(mtx::events::msg::KeyVerificationReady)
MTXCLIENT_SEND_TO_DEVICE(mtx::events::msg::KeyVerificationDone)
MTXCLIENT_SEND_TO_DEVICE(mtx::events::msg::KeyVerificationAccept)
MTXCLIENT_SEND_TO_DEVICE(mtx::events::msg::KeyVerificationCancel)
MTXCLIENT_SEND_TO_DEVICE(mtx::events::msg::KeyVerificationKey)
MTXCLIENT_SEND_TO_DEVICE(mtx::events::msg::KeyVerificationMac)
MTXCLIENT_SEND_TO_DEVICE(mtx::events::msg::SecretSend)
MTXCLIENT_SEND_TO_DEVICE(mtx::events::msg::SecretRequest)

#define MTXCLIENT_ACCOUNT_DATA(Payload)                                                            \
    template void mtx::http::Client::put_room_account_data<Payload>(const std::string &room_id,    \
                                                                    const std::string &type,       \
                                                                    const Payload &payload,        \
                                                                    ErrCallback cb);               \
    template void mtx::http::Client::put_room_account_data<Payload>(                               \
      const std::string &room_id, const Payload &payload, ErrCallback cb);                         \
    template void mtx::http::Client::put_account_data<Payload>(                                    \
      const std::string &type, const Payload &payload, ErrCallback cb);                            \
    template void mtx::http::Client::put_account_data<Payload>(const Payload &payload,             \
                                                               ErrCallback cb);                    \
    template void mtx::http::Client::get_room_account_data<Payload>(                               \
      const std::string &room_id, const std::string &type, Callback<Payload> payload);             \
    template void mtx::http::Client::get_room_account_data<Payload>(const std::string &room_id,    \
                                                                    Callback<Payload> cb);         \
    template void mtx::http::Client::get_account_data<Payload>(const std::string &type,            \
                                                               Callback<Payload> payload);         \
    template void mtx::http::Client::get_account_data<Payload>(Callback<Payload> cb);

MTXCLIENT_ACCOUNT_DATA(mtx::events::msc2545::ImagePack)
MTXCLIENT_ACCOUNT_DATA(mtx::events::msc2545::ImagePackRooms)
MTXCLIENT_ACCOUNT_DATA(mtx::events::account_data::nheko_extensions::HiddenEvents)
MTXCLIENT_ACCOUNT_DATA(mtx::events::account_data::Tags)
MTXCLIENT_ACCOUNT_DATA(mtx::events::account_data::Direct)
