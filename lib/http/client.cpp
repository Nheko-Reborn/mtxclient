#include <boost/algorithm/string.hpp>
#include <boost/bind.hpp>
#include <boost/utility/typed_in_place_factory.hpp>

#include "mtxclient/http/client.hpp"
#include "mtxclient/utils.hpp"

#include "mtx/requests.hpp"
#include "mtx/responses.hpp"

using namespace mtx::http;
using namespace boost::beast;

Client::Client(const std::string &server, uint16_t port)
  : server_{server}
  , port_{port}
{
        using namespace boost::asio;
        const auto threads_num = std::max(1U, std::thread::hardware_concurrency());

        for (unsigned int i = 0; i < threads_num; ++i)
                thread_group_.add_thread(new boost::thread([this]() { ios_.run(); }));
}

void
Client::set_server(const std::string &server)
{
        // Check if the input also contains the port.
        std::vector<std::string> parts;
        boost::split(parts, server, [](char c) { return c == ':'; });

        if (parts.size() == 2 && mtx::client::utils::is_number(parts.at(1))) {
                server_ = parts.at(0);
                port_   = std::stoi(parts.at(1));
        } else {
                server_ = server;
        }
}

void
Client::close()
{
        // Destroy work object. This allows the I/O thread to
        // exit the event loop when there are no more pending
        // asynchronous operations.
        work_.reset();

        // Wait for the worker threads to exit.
        thread_group_.join_all();
}

void
Client::setup_auth(Session *session, bool auth)
{
        const auto token = access_token();

        if (auth && !token.empty())
                session->request.set(boost::beast::http::field::authorization, "Bearer " + token);
}

//
// Client API endpoints
//

void
Client::login(const std::string &user,
              const std::string &password,
              Callback<mtx::responses::Login> callback)
{
        mtx::requests::Login req;
        req.user     = user;
        req.password = password;

        post<mtx::requests::Login, mtx::responses::Login>(
          "/client/r0/login",
          req,
          [_this = shared_from_this(), callback](const mtx::responses::Login &resp,
                                                 RequestErr err) {
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
Client::logout(Callback<mtx::responses::Logout> callback)
{
        mtx::requests::Logout req;

        post<mtx::requests::Logout, mtx::responses::Logout>(
          "/client/r0/logout",
          req,
          [_this = shared_from_this(), callback](const mtx::responses::Logout &res,
                                                 RequestErr err) {
                  if (!err) {
                          // Clear the now invalid access token when logout is successful
                          _this->access_token_.clear();
                  }
                  // Pass up response and error to supplied callback
                  callback(res, err);
          });
}

void
Client::set_avatar_url(const std::string &avatar_url, ErrCallback callback)
{
        mtx::requests::AvatarUrl req;
        req.avatar_url = avatar_url;

        put<mtx::requests::AvatarUrl>(
          "/client/r0/profile/" + user_id_.to_string() + "/avatar_url", req, callback);
}

void
Client::set_displayname(const std::string &displayname, ErrCallback callback)
{
        mtx::requests::DisplayName req;
        req.displayname = displayname;

        put<mtx::requests::DisplayName>(
          "/client/r0/profile/" + user_id_.to_string() + "/displayname", req, callback);
}

void
Client::get_profile(const std::string &user_id, Callback<mtx::responses::Profile> callback)
{
        get<mtx::responses::Profile>("/client/r0/profile/" + user_id,
                                     [callback](const mtx::responses::Profile &res,
                                                HeaderFields,
                                                RequestErr err) { callback(res, err); });
}

void
Client::get_avatar_url(const std::string &user_id, Callback<mtx::responses::AvatarUrl> callback)
{
        get<mtx::responses::AvatarUrl>("/client/r0/profile/" + user_id + "/avatar_url",
                                       [callback](const mtx::responses::AvatarUrl &res,
                                                  HeaderFields,
                                                  RequestErr err) { callback(res, err); });
}

void
Client::create_room(const mtx::requests::CreateRoom &room_options,
                    Callback<mtx::responses::CreateRoom> callback)
{
        post<mtx::requests::CreateRoom, mtx::responses::CreateRoom>(
          "/client/r0/createRoom", room_options, callback);
}

void
Client::join_room(const std::string &room, Callback<nlohmann::json> callback)
{
        auto api_path = "/client/r0/join/" + room;

        post<std::string, nlohmann::json>(api_path, "", callback);
}

void
Client::leave_room(const std::string &room_id, Callback<nlohmann::json> callback)
{
        auto api_path = "/client/r0/rooms/" + room_id + "/leave";

        post<std::string, nlohmann::json>(api_path, "", callback);
}

void
Client::invite_user(const std::string &room_id,
                    const std::string &user_id,
                    Callback<mtx::responses::RoomInvite> callback)
{
        mtx::requests::RoomInvite req;
        req.user_id = user_id;

        auto api_path = "/client/r0/rooms/" + room_id + "/invite";

        post<mtx::requests::RoomInvite, mtx::responses::RoomInvite>(api_path, req, callback);
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

        params.emplace("timeout", std::to_string(opts.timeout));

        get<mtx::responses::Sync>("/client/r0/sync?" + mtx::client::utils::query_params(params),
                                  [callback](const mtx::responses::Sync &res,
                                             HeaderFields,
                                             RequestErr err) { callback(res, err); });
}

void
Client::versions(Callback<mtx::responses::Versions> callback)
{
        get<mtx::responses::Versions>("/client/versions",
                                      [callback](const mtx::responses::Versions &res,
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

        const auto api_path = "/media/r0/upload?" + client::utils::query_params(params);
        post<std::string, mtx::responses::ContentURI>(api_path, data, cb, true, content_type);
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
Client::download(const std::string &server,
                 const std::string &media_id,
                 std::function<void(const std::string &res,
                                    const std::string &content_type,
                                    const std::string &original_filename,
                                    RequestErr err)> callback)
{
        const auto api_path = "/media/r0/download/" + server + "/" + media_id;
        get<std::string>(
          api_path, [callback](const std::string &res, HeaderFields fields, RequestErr err) {
                  std::string content_type, original_filename;

                  if (fields) {
                          if (fields->find("Content-Type") != fields->end())
                                  content_type = fields->at("Content-Type").to_string();
                          if (fields->find("Content-Disposition") != fields->end()) {
                                  auto value = fields->at("Content-Disposition").to_string();

                                  std::vector<std::string> results;
                                  boost::split(results, value, [](char c) { return c == '='; });

                                  original_filename = results.back();
                          }
                  }

                  callback(res, content_type, original_filename, err);
          });
}

void
Client::start_typing(const std::string &room_id, uint64_t timeout, ErrCallback callback)
{
        const auto api_path = "/client/r0/rooms/" + room_id + "/typing/" + user_id_.to_string();

        mtx::requests::TypingNotification req;
        req.typing  = true;
        req.timeout = timeout;

        put<mtx::requests::TypingNotification>(api_path, req, callback);
}

void
Client::stop_typing(const std::string &room_id, ErrCallback callback)
{
        const auto api_path = "/client/r0/rooms/" + room_id + "/typing/" + user_id_.to_string();

        mtx::requests::TypingNotification req;
        req.typing = false;

        put<mtx::requests::TypingNotification>(api_path, req, callback);
}

void
Client::messages(const std::string &room_id,
                 const std::string &from,
                 const std::string &to,
                 PaginationDirection dir,
                 uint16_t limit,
                 const std::string &filter,
                 Callback<mtx::responses::Messages> callback)
{
        std::map<std::string, std::string> params;

        params.emplace("from", from);
        params.emplace("dir", to_string(dir));

        if (!to.empty())
                params.emplace("to", to);
        if (limit > 0)
                params.emplace("limit", std::to_string(limit));
        if (!filter.empty())
                params.emplace("filter", filter);

        const auto api_path =
          "/client/r0/rooms/" + room_id + "/messages?" + client::utils::query_params(params);

        get<mtx::responses::Messages>(
          api_path, [callback](const mtx::responses::Messages &res, HeaderFields, RequestErr err) {
                  callback(res, err);
          });
}

void
Client::upload_filter(const nlohmann::json &j, Callback<mtx::responses::FilterId> callback)
{
        const auto api_path = "/client/r0/user/" + user_id_.to_string() + "/filter";

        post<nlohmann::json, mtx::responses::FilterId>(api_path, j, callback);
}

void
Client::read_event(const std::string &room_id, const std::string &event_id, ErrCallback callback)
{
        const auto api_path = "/client/r0/rooms/" + room_id + "/read_markers";

        nlohmann::json body = {{"m.fully_read", event_id}, {"m.read", event_id}};

        post<nlohmann::json, mtx::responses::Empty>(
          api_path, body, [callback](const mtx::responses::Empty, RequestErr err) {
                  callback(err);
          });
}

void
Client::registration(const std::string &user,
                     const std::string &pass,
                     Callback<mtx::responses::Register> callback)
{
        nlohmann::json req = {{"username", user}, {"password", pass}};

        post<nlohmann::json, mtx::responses::Register>("/client/r0/register", req, callback, false);
}

void
Client::flow_register(const std::string &user,
                      const std::string &pass,
                      Callback<mtx::responses::RegistrationFlows> callback)
{
        nlohmann::json req = {{"username", user}, {"password", pass}};

        post<nlohmann::json, mtx::responses::RegistrationFlows>(
          "/client/r0/register", req, callback, false);
}

void
Client::flow_response(const std::string &user,
                      const std::string &pass,
                      const std::string &session,
                      const std::string &flow_type,
                      Callback<mtx::responses::Register> callback)
{
        nlohmann::json req = {{"username", user},
                              {"password", pass},
                              {"auth", {{"type", flow_type}, {"session", session}}}};

        post<nlohmann::json, mtx::responses::Register>("/client/r0/register", req, callback, false);
}

void
Client::send_to_device(const std::string &event_type,
                       const std::string &txid,
                       const nlohmann::json &body,
                       ErrCallback callback)
{
        const auto api_path = "/client/r0/sendToDevice/" + event_type + "/" + txid;
        put<nlohmann::json>(api_path, body, callback);
}

//
// Encryption related endpoints
//

void
Client::upload_keys(const mtx::requests::UploadKeys &req,
                    Callback<mtx::responses::UploadKeys> callback)
{
        post<mtx::requests::UploadKeys, mtx::responses::UploadKeys>(
          "/client/r0/keys/upload", req, callback);
}

void
Client::query_keys(const mtx::requests::QueryKeys &req,
                   Callback<mtx::responses::QueryKeys> callback)
{
        post<mtx::requests::QueryKeys, mtx::responses::QueryKeys>(
          "/client/r0/keys/query", req, callback);
}

//! Claims one-time keys for use in pre-key messages.
void
Client::claim_keys(const std::string &user,
                   const std::vector<std::string> &devices,
                   Callback<mtx::responses::ClaimKeys> cb)
{
        mtx::requests::ClaimKeys req;

        std::map<std::string, std::string> dev_to_algorithm;
        for (const auto &d : devices)
                dev_to_algorithm.emplace(d, "signed_curve25519");

        req.one_time_keys[user] = dev_to_algorithm;

        post<mtx::requests::ClaimKeys, mtx::responses::ClaimKeys>(
          "/client/r0/keys/claim", std::move(req), std::move(cb));
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
          "/client/r0/keys/changes?" + mtx::client::utils::query_params(params),
          [callback](const mtx::responses::KeyChanges &res, HeaderFields, RequestErr err) {
                  callback(res, err);
          });
}

void
Client::enable_encryption(const std::string &room, Callback<mtx::responses::EventId> callback)
{
        using namespace mtx::events;
        state::Encryption event;

        send_state_event<state::Encryption, EventType::RoomEncryption>(room, "", event, callback);
}
