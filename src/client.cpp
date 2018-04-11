#include <boost/algorithm/string.hpp>
#include <boost/bind.hpp>
#include <boost/utility/typed_in_place_factory.hpp>

#include "client.hpp"
#include "utils.hpp"

#include "mtx/requests.hpp"
#include "mtx/responses.hpp"

using namespace mtx::client;
using namespace boost::beast;

Client::Client(const std::string &server, uint16_t port)
  : resolver_{ios_}
  , server_{server}
  , port_{port}
{
        using namespace boost::asio;
        work_ = boost::in_place<io_service::work>(io_service::work(ios_));

        const auto threads_num = std::max(1U, std::thread::hardware_concurrency());

        for (unsigned int i = 0; i < threads_num; ++i)
                thread_group_.add_thread(new boost::thread([this]() { ios_.run(); }));
}

void
Client::close()
{
        // Destroy work object. This allows the I/O thread to
        // exit the event loop when there are no more pending
        // asynchronous operations.
        work_ = boost::none;

        // Wait for the worker threads to exit.
        thread_group_.join_all();
}

void
Client::on_resolve(std::shared_ptr<Session> s,
                   boost::system::error_code ec,
                   boost::asio::ip::tcp::resolver::results_type results)
{
        if (ec)
                return s->on_failure(s->id, ec);

        boost::asio::async_connect(
          s->socket.next_layer(),
          results.begin(),
          results.end(),
          std::bind(&Client::on_connect, shared_from_this(), s, std::placeholders::_1));
}

void
Client::on_connect(std::shared_ptr<Session> s, boost::system::error_code ec)
{
        if (ec) {
                remove_session(s);
                return s->on_failure(s->id, ec);
        }

        // Perform the SSL handshake
        s->socket.async_handshake(
          boost::asio::ssl::stream_base::client,
          std::bind(&Client::on_handshake, shared_from_this(), s, std::placeholders::_1));
}

void
Client::on_handshake(std::shared_ptr<Session> s, boost::system::error_code ec)
{
        if (ec) {
                remove_session(s);
                return s->on_failure(s->id, ec);
        }

        boost::beast::http::async_write(s->socket,
                                        s->request,
                                        std::bind(&Client::on_write,
                                                  shared_from_this(),
                                                  s,
                                                  std::placeholders::_1,
                                                  std::placeholders::_2));
}

void
Client::on_write(std::shared_ptr<Session> s,
                 boost::system::error_code ec,
                 std::size_t bytes_transferred)
{
        boost::ignore_unused(bytes_transferred);

        if (ec) {
                remove_session(s);
                return s->on_failure(s->id, ec);
        }

        // Receive the HTTP response
        http::async_read(
          s->socket,
          s->output_buf,
          s->parser,
          std::bind(
            &Client::on_read, shared_from_this(), s, std::placeholders::_1, std::placeholders::_2));
}

void
Client::on_read(std::shared_ptr<Session> s,
                boost::system::error_code ec,
                std::size_t bytes_transferred)
{
        boost::ignore_unused(bytes_transferred);

        if (ec)
                s->error_code = ec;

        on_request_complete(s);
}

void
Client::do_request(std::shared_ptr<Session> s)
{
        resolver_.async_resolve(server_,
                                std::to_string(port_),
                                std::bind(&Client::on_resolve,
                                          shared_from_this(),
                                          s,
                                          std::placeholders::_1,
                                          std::placeholders::_2));
}

void
Client::remove_session(std::shared_ptr<Session> s)
{
        // Shutting down the connection. This method may
        // fail in case the socket is not connected. We don't
        // care about the error code if this function fails.
        boost::system::error_code ignored_ec;

        s->socket.async_shutdown([s](boost::system::error_code ec) {
                if (ec == boost::asio::error::eof) {
                        // Rationale:
                        // http://stackoverflow.com/questions/25587403/boost-asio-ssl-async-shutdown-always-finishes-with-an-error
                        ec.assign(0, ec.category());
                }

                // Ignoring short_read error.
                if ((ec.category() == boost::asio::error::get_ssl_category()) &&
                    (ERR_GET_REASON(ec.value()) == SSL_R_SHORT_READ))
                        return;

                if (ec)
                        // TODO: propagate the error.
                        std::cout << "shutdown: " << ec.message() << std::endl;
        });
}

void
Client::on_request_complete(std::shared_ptr<Session> s)
{
        remove_session(s);

        boost::system::error_code ec(s->error_code);
        s->on_success(s->id, s->parser.get(), ec);
}

void
Client::setup_auth(std::shared_ptr<Session> session, bool auth)
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
              std::function<void(const mtx::responses::Login &response, RequestErr err)> callback)
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
Client::logout(std::function<void(const mtx::responses::Logout &response, RequestErr)> callback)
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
Client::set_avatar_url(const std::string &avatar_url, std::function<void(RequestErr err)> callback)
{
        mtx::requests::AvatarUrl req;
        req.avatar_url = avatar_url;

        put<mtx::requests::AvatarUrl>(
          "/client/r0/profile/" + user_id_.to_string() + "/avatar_url", req, callback);
}

void
Client::set_displayname(const std::string &displayname, std::function<void(RequestErr)> callback)
{
        mtx::requests::DisplayName req;
        req.displayname = displayname;

        put<mtx::requests::DisplayName>(
          "/client/r0/profile/" + user_id_.to_string() + "/displayname", req, callback);
}

void
Client::get_profile(const mtx::identifiers::User &user_id,
                    std::function<void(const mtx::responses::Profile &, RequestErr)> callback)
{
        get<mtx::responses::Profile>("/client/r0/profile/" + user_id.to_string(),
                                     [callback](const mtx::responses::Profile &res,
                                                HeaderFields,
                                                RequestErr err) { callback(res, err); });
}

void
Client::get_avatar_url(const mtx::identifiers::User &user_id,
                       std::function<void(const mtx::responses::AvatarUrl &, RequestErr)> callback)
{
        get<mtx::responses::AvatarUrl>("/client/r0/profile/" + user_id.to_string() + "/avatar_url",
                                       [callback](const mtx::responses::AvatarUrl &res,
                                                  HeaderFields,
                                                  RequestErr err) { callback(res, err); });
}

void
Client::create_room(const mtx::requests::CreateRoom &room_options,
                    std::function<void(const mtx::responses::CreateRoom &, RequestErr)> callback)
{
        post<mtx::requests::CreateRoom, mtx::responses::CreateRoom>(
          "/client/r0/createRoom", room_options, callback);
}

void
Client::join_room(const mtx::identifiers::Room &room_id,
                  std::function<void(const nlohmann::json &, RequestErr)> callback)
{
        auto api_path = "/client/r0/rooms/" + room_id.to_string() + "/join";

        post<std::string, nlohmann::json>(api_path, "", callback);
}

void
Client::join_room(const std::string &room,
                  std::function<void(const nlohmann::json &, RequestErr)> callback)
{
        auto api_path = "/client/r0/join/" + room;

        post<std::string, nlohmann::json>(api_path, "", callback);
}

void
Client::leave_room(const mtx::identifiers::Room &room_id,
                   std::function<void(const nlohmann::json &, RequestErr)> callback)
{
        auto api_path = "/client/r0/rooms/" + room_id.to_string() + "/leave";

        post<std::string, nlohmann::json>(api_path, "", callback);
}

void
Client::invite_user(const mtx::identifiers::Room &room_id,
                    const std::string &user_id,
                    std::function<void(const mtx::responses::RoomInvite &, RequestErr)> callback)
{
        mtx::requests::RoomInvite req;
        req.user_id = user_id;

        auto api_path = "/client/r0/rooms/" + room_id.to_string() + "/invite";

        post<mtx::requests::RoomInvite, mtx::responses::RoomInvite>(api_path, req, callback);
}

void
Client::sync(const std::string &filter,
             const std::string &since,
             bool full_state,
             uint16_t timeout,
             std::function<void(const nlohmann::json &, RequestErr)> callback)
{
        std::map<std::string, std::string> params;

        if (!filter.empty())
                params.emplace("filter", filter);

        if (!since.empty())
                params.emplace("since", since);

        if (full_state)
                params.emplace("full_state", "true");

        params.emplace("timeout", std::to_string(timeout));

        get<nlohmann::json>("/client/r0/sync?" + utils::query_params(params),
                            [callback](const nlohmann::json &res, HeaderFields, RequestErr err) {
                                    callback(res, err);
                            });
}

void
Client::versions(std::function<void(const mtx::responses::Versions &, RequestErr)> callback)
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
               std::function<void(const mtx::responses::ContentURI &res, RequestErr err)> cb)
{
        std::map<std::string, std::string> params = {{"filename", filename}};

        const auto api_path = "/media/r0/upload?" + utils::query_params(params);
        post<std::string, mtx::responses::ContentURI>(api_path, data, cb, true, content_type);
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
Client::start_typing(const mtx::identifiers::Room &room_id,
                     uint64_t timeout,
                     std::function<void(RequestErr)> callback)
{
        const auto api_path =
          "/client/r0/rooms/" + room_id.to_string() + "/typing/" + user_id_.to_string();

        mtx::requests::TypingNotification req;
        req.typing  = true;
        req.timeout = timeout;

        put<mtx::requests::TypingNotification>(api_path, req, callback);
}

void
Client::stop_typing(const mtx::identifiers::Room &room_id, std::function<void(RequestErr)> callback)
{
        const auto api_path =
          "/client/r0/rooms/" + room_id.to_string() + "/typing/" + user_id_.to_string();

        mtx::requests::TypingNotification req;
        req.typing = false;

        put<mtx::requests::TypingNotification>(api_path, req, callback);
}

void
Client::messages(const mtx::identifiers::Room &room_id,
                 const std::string &from,
                 const std::string &to,
                 PaginationDirection dir,
                 uint16_t limit,
                 const std::string &filter,
                 std::function<void(const mtx::responses::Messages &res, RequestErr err)> callback)
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
          "/client/r0/rooms/" + room_id.to_string() + "/messages?" + utils::query_params(params);

        get<mtx::responses::Messages>(
          api_path, [callback](const mtx::responses::Messages &res, HeaderFields, RequestErr err) {
                  callback(res, err);
          });
}

void
Client::upload_filter(const nlohmann::json &j,
                      std::function<void(const mtx::responses::FilterId, RequestErr err)> callback)
{
        const auto api_path = "/client/r0/user/" + user_id_.to_string() + "/filter";

        post<nlohmann::json, mtx::responses::FilterId>(api_path, j, callback);
}

void
Client::read_event(const mtx::identifiers::Room &room_id,
                   const mtx::identifiers::Event &event_id,
                   std::function<void(RequestErr err)> callback)
{
        const auto api_path = "/client/r0/rooms/" + room_id.to_string() + "/read_markers";

        nlohmann::json body = {{"m.fully_read", event_id.to_string()},
                               {"m.read", event_id.to_string()}};

        post<nlohmann::json, mtx::responses::Empty>(
          api_path, body, [callback](const mtx::responses::Empty, RequestErr err) {
                  callback(err);
          });
}

void
Client::registration(const std::string &user,
                     const std::string &pass,
                     std::function<void(const mtx::responses::Register &, RequestErr)> callback)
{
        nlohmann::json req = {{"username", user}, {"password", pass}};

        post<nlohmann::json, mtx::responses::Register>("/client/r0/register", req, callback, false);
}

void
Client::flow_register(
  const std::string &user,
  const std::string &pass,
  std::function<void(const mtx::responses::RegistrationFlows &, RequestErr)> callback)
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
                      std::function<void(const mtx::responses::Register &, RequestErr)> callback)
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
                       std::function<void(RequestErr)> callback)
{
        const auto api_path = "/client/r0/sendToDevice/" + event_type + "/" + txid;
        put<nlohmann::json>(api_path, body, callback);
}

//
// Encryption related endpoints
//

void
Client::upload_keys(
  const mtx::requests::UploadKeys &req,
  std::function<void(const mtx::responses::UploadKeys &res, RequestErr err)> callback)
{
        post<mtx::requests::UploadKeys, mtx::responses::UploadKeys>(
          "/client/r0/keys/upload", req, callback);
}

void
Client::query_keys(
  const mtx::requests::QueryKeys &req,
  std::function<void(const mtx::responses::QueryKeys &res, RequestErr err)> callback)
{
        post<mtx::requests::QueryKeys, mtx::responses::QueryKeys>(
          "/client/r0/keys/query", req, callback);
}

//! Claims one-time keys for use in pre-key messages.
void
Client::claim_keys(const mtx::identifiers::User &user,
                   const std::vector<std::string> &devices,
                   std::function<void(const mtx::responses::ClaimKeys &res, RequestErr err)> cb)
{
        mtx::requests::ClaimKeys req;

        std::map<std::string, std::string> dev_to_algorithm;
        for (const auto &d : devices)
                dev_to_algorithm.emplace(d, "signed_curve25519");

        req.one_time_keys[user.to_string()] = dev_to_algorithm;

        post<mtx::requests::ClaimKeys, mtx::responses::ClaimKeys>(
          "/client/r0/keys/claim", std::move(req), std::move(cb));
}

void
Client::key_changes(
  const std::string &from,
  const std::string &to,
  std::function<void(const mtx::responses::KeyChanges &res, RequestErr err)> callback)
{
        std::map<std::string, std::string> params;

        if (!from.empty())
                params.emplace("from", from);

        if (!to.empty())
                params.emplace("to", to);

        get<mtx::responses::KeyChanges>("/client/r0/keys/changes?" + utils::query_params(params),
                                        [callback](const mtx::responses::KeyChanges &res,
                                                   HeaderFields,
                                                   RequestErr err) { callback(res, err); });
}

void
Client::enable_encryption(const mtx::identifiers::Room &room,
                          std::function<void(const mtx::responses::EventId &, RequestErr)> callback)
{
        using namespace mtx::events;
        state::Encryption event;

        send_state_event<state::Encryption, EventType::RoomEncryption>(room, "", event, callback);
}
