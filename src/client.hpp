#pragma once

#include <memory>
#include <mutex>
#include <thread>

#include <boost/asio.hpp>
#include <boost/asio/ssl.hpp>
#include <boost/beast.hpp>
#include <boost/iostreams/stream.hpp>
#include <boost/optional.hpp>
#include <boost/thread/thread.hpp>
#include <json.hpp>

#include <mtx/requests.hpp>
#include <mtx/responses.hpp>

#include "crypto.hpp"
#include "errors.hpp"
#include "session.hpp"
#include "utils.hpp"

namespace mtx {
namespace client {

enum class PaginationDirection
{
        Backwards,
        Forwards,
};

inline std::string
to_string(PaginationDirection dir)
{
        if (dir == PaginationDirection::Backwards)
                return "b";

        return "f";
}

using RequestErr = const boost::optional<mtx::client::errors::ClientError> &;

//! The main object that the user will interact.
class Client : public std::enable_shared_from_this<Client>
{
public:
        Client(const std::string &server = "", uint16_t port = 443);

        //! Wait for the client to close.
        void close();
        //! Make a new request.
        void do_request(std::shared_ptr<Session> session);
        //! Add an access token.
        void set_access_token(const std::string &token) { access_token_ = token; }
        //! Retrieve the access token.
        std::string access_token() const { return access_token_; }
        //! Update the next batch token.
        void set_next_batch_token(const std::string &token) { next_batch_token_ = token; }
        //! Retrieve the current next batch token.
        std::string next_batch_token() const { return next_batch_token_; }
        //! Retrieve the user_id.
        mtx::identifiers::User user_id() const { return user_id_; }
        //! Retrieve the device_id.
        std::string device_id() const { return device_id_; }
        //! Generate a new transaction id.
        std::string generate_txn_id() { return utils::random_token(32, false); }

        using HeaderFields = const boost::optional<boost::beast::http::fields> &;

        //! Perfom login.
        void login(const std::string &username,
                   const std::string &password,
                   std::function<void(const mtx::responses::Login &response, RequestErr err)>);

        //! Register by not expecting a registration flow.
        void registration(const std::string &user,
                          const std::string &pass,
                          std::function<void(const mtx::responses::Register &, RequestErr)>);

        //! Register through a registration flow.
        void flow_register(
          const std::string &user,
          const std::string &pass,
          std::function<void(const mtx::responses::RegistrationFlows &, RequestErr)>);

        //! Complete the flow registration.
        void flow_response(const std::string &user,
                           const std::string &pass,
                           const std::string &session,
                           const std::string &flow_type,
                           std::function<void(const mtx::responses::Register &, RequestErr)>);

        //! Perform logout.
        void logout(std::function<void(const mtx::responses::Logout &response, RequestErr err)>);
        //! Change avatar.
        void set_avatar_url(const std::string &avatar_url, std::function<void(RequestErr err)>);
        //! Change displayname.
        void set_displayname(const std::string &displayname, std::function<void(RequestErr err)>);
        //! Get user profile.
        void get_profile(const mtx::identifiers::User &user_id,
                         std::function<void(const mtx::responses::Profile &, RequestErr)> callback);
        //! Get user avatar URL.
        void get_avatar_url(
          const mtx::identifiers::User &user_id,
          std::function<void(const mtx::responses::AvatarUrl &, RequestErr)> callback);
        //! Create a room with the given options.
        void create_room(
          const mtx::requests::CreateRoom &room_options,
          std::function<void(const mtx::responses::CreateRoom &response, RequestErr err)>);
        //! Join a room by its room_id.
        void join_room(const mtx::identifiers::Room &room_id,
                       std::function<void(const nlohmann::json &res, RequestErr err)>);
        //! Join a room by an alias or a room_id.
        void join_room(const std::string &room,
                       std::function<void(const nlohmann::json &res, RequestErr err)>);
        //! Leave a room by its room_id.
        void leave_room(const mtx::identifiers::Room &room_id,
                        std::function<void(const nlohmann::json &res, RequestErr err)>);
        //! Invite a user to a room.
        void invite_user(
          const mtx::identifiers::Room &room_id,
          const std::string &user_id,
          std::function<void(const mtx::responses::RoomInvite &res, RequestErr err)>);
        //! Perform sync.
        void sync(const std::string &filter,
                  const std::string &since,
                  bool full_state,
                  uint16_t timeout,
                  std::function<void(const nlohmann::json &res, RequestErr err)>);

        //! Paginate through room messages.
        void messages(const mtx::identifiers::Room &room_id,
                      const std::string &from,
                      const std::string &to,
                      PaginationDirection dir,
                      uint16_t limit,
                      const std::string &filter,
                      std::function<void(const mtx::responses::Messages &res, RequestErr err)>);

        //! Get the supported versions from the server.
        void versions(std::function<void(const mtx::responses::Versions &res, RequestErr err)>);

        //! Mark an event as read.
        void read_event(const mtx::identifiers::Room &room_id,
                        const mtx::identifiers::Event &event_id,
                        std::function<void(RequestErr err)>);

        //! Upload a filter
        void upload_filter(const nlohmann::json &j,
                           std::function<void(const mtx::responses::FilterId, RequestErr err)>);

        //! Upload data to the content repository.
        void upload(const std::string &data,
                    const std::string &content_type,
                    const std::string &filename,
                    std::function<void(const mtx::responses::ContentURI &res, RequestErr err)> cb);
        //! Retrieve data from the content repository.
        void download(const std::string &server,
                      const std::string &media_id,
                      std::function<void(const std::string &data,
                                         const std::string &content_type,
                                         const std::string &original_filename,
                                         RequestErr err)> cb);
        //! Send typing notifications to the room.
        void start_typing(const mtx::identifiers::Room &room_id,
                          uint64_t timeout,
                          std::function<void(RequestErr err)> cb);
        //! Remove typing notifications from the room.
        void stop_typing(const mtx::identifiers::Room &room_id,
                         std::function<void(RequestErr err)> cb);
        //! Send a room message with auto-generated transaction id.
        template<class Payload, mtx::events::EventType Event>
        void send_room_message(
          const mtx::identifiers::Room &room_id,
          const Payload &payload,
          std::function<void(const mtx::responses::EventId &, RequestErr)> callback);
        //! Send a room message by providing transaction id.
        template<class Payload, mtx::events::EventType Event>
        void send_room_message(
          const mtx::identifiers::Room &room_id,
          const std::string &txn_id,
          const Payload &payload,
          std::function<void(const mtx::responses::EventId &, RequestErr)> callback);
        //! Send a state event by providing the state key.
        template<class Payload, mtx::events::EventType Event>
        void send_state_event(
          const mtx::identifiers::Room &room_id,
          const std::string &state_key,
          const Payload &payload,
          std::function<void(const mtx::responses::EventId &, RequestErr)> callback);
        //! Send a state event with an empty state key.
        template<class Payload, mtx::events::EventType Event>
        void send_state_event(
          const mtx::identifiers::Room &room_id,
          const Payload &payload,
          std::function<void(const mtx::responses::EventId &, RequestErr)> callback);

        //
        // Encryption related endpoints.
        //

        //! Upload identity keys & one time keys.
        void upload_keys(
          const mtx::requests::UploadKeys &req,
          std::function<void(const mtx::responses::UploadKeys &res, RequestErr err)> cb);

        //! Returns the current devices and identity keys for the given users.
        void query_keys(
          const mtx::requests::QueryKeys &req,
          std::function<void(const mtx::responses::QueryKeys &res, RequestErr err)> cb);

        //! Gets a list of users who have updated their device identity keys
        //! since a previous sync token.
        void key_changes(
          const std::string &from,
          const std::string &to,
          std::function<void(const mtx::responses::KeyChanges &res, RequestErr err)> cb);

        //! Enable encryption in a room by sending a `m.room.encryption` state event.
        void enable_encryption(
          const mtx::identifiers::Room &room,
          std::function<void(const mtx::responses::EventId &res, RequestErr err)>);

private:
        template<class Request, class Response>
        void post(const std::string &endpoint,
                  const Request &req,
                  std::function<void(const Response &, RequestErr)>,
                  bool requires_auth              = true,
                  const std::string &content_type = "application/json");

        // put function for the PUT HTTP requests that send responses
        template<class Request, class Response>
        void put(const std::string &endpoint,
                 const Request &req,
                 std::function<void(const Response &, RequestErr)>,
                 bool requires_auth = true);

        template<class Request>
        void put(const std::string &endpoint,
                 const Request &req,
                 std::function<void(RequestErr err)>,
                 bool requires_auth = true);

        template<class Response>
        void get(const std::string &endpoint,
                 std::function<void(const Response &res, HeaderFields fields, RequestErr err)>,
                 bool requires_auth = true);

        template<class Response>
        std::shared_ptr<Session> create_session(
          std::function<void(const Response &res, HeaderFields fields, RequestErr err)> callback);

        //! Setup http header with the access token if needed.
        void setup_auth(std::shared_ptr<Session> session, bool auth);

        void remove_session(std::shared_ptr<Session> s);
        void on_request_complete(std::shared_ptr<Session> s);
        void on_resolve(std::shared_ptr<Session> s,
                        boost::system::error_code ec,
                        boost::asio::ip::tcp::resolver::results_type results);
        void on_connect(std::shared_ptr<Session> s, boost::system::error_code ec);
        void on_handshake(std::shared_ptr<Session> s, boost::system::error_code ec);
        void on_write(std::shared_ptr<Session> s,
                      boost::system::error_code ec,
                      std::size_t bytes_transferred);
        void on_read(std::shared_ptr<Session> s,
                     boost::system::error_code ec,
                     std::size_t bytes_transferred);

        boost::asio::io_service ios_;

        //! Used to prevent the event loop from shutting down.
        std::unique_ptr<boost::asio::io_service::work> work_;
        //! Worker threads for the requests.
        boost::thread_group thread_group_;
        //! Used to resolve DNS names.
        boost::asio::ip::tcp::resolver resolver_;
        //! The homeserver to connect to.
        std::string server_;
        //! The access token that would be used for authentication.
        std::string access_token_;
        //! The user ID associated with the client.
        mtx::identifiers::User user_id_;
        //! The device that this session is associated with.
        std::string device_id_;
        //! The token that will be used as the 'since' parameter on the next sync request.
        std::string next_batch_token_;
        //! The homeserver port to connect.
        uint16_t port_ = 443;
};
}
}

template<class T>
inline T
deserialize(const std::string &data)
{
        return json::parse(data);
}

template<>
inline std::string
deserialize<std::string>(const std::string &data)
{
        return data;
}

template<class T>
inline std::string
serialize(const T &obj)
{
        return json(obj).dump();
}

template<>
inline std::string
serialize<std::string>(const std::string &obj)
{
        return obj;
}

template<class Request, class Response>
void
mtx::client::Client::post(const std::string &endpoint,
                          const Request &req,
                          std::function<void(const Response &, RequestErr)> callback,
                          bool requires_auth,
                          const std::string &content_type)
{
        std::shared_ptr<Session> session = create_session<Response>(
          [callback](const Response &res, HeaderFields, RequestErr err) { callback(res, err); });

        if (!session)
                return;

        setup_auth(session, requires_auth);

        session->request.method(boost::beast::http::verb::post);
        session->request.target("/_matrix" + endpoint);
        session->request.set(boost::beast::http::field::user_agent, "mtxclient v0.1.0");
        session->request.set(boost::beast::http::field::content_type, content_type);
        session->request.set(boost::beast::http::field::accept_encoding, "gzip,deflate");
        session->request.set(boost::beast::http::field::host, session->host);
        session->request.body() = serialize<Request>(req);
        session->request.prepare_payload();

        do_request(session);
}

// put function for the PUT HTTP requests that send responses
template<class Request, class Response>
void
mtx::client::Client::put(const std::string &endpoint,
                         const Request &req,
                         std::function<void(const Response &, RequestErr)> callback,
                         bool requires_auth)
{
        std::shared_ptr<Session> session = create_session<Response>(
          [callback](const Response &res, HeaderFields, RequestErr err) { callback(res, err); });

        if (!session)
                return;

        setup_auth(session, requires_auth);

        session->request.method(boost::beast::http::verb::put);
        session->request.target("/_matrix" + endpoint);
        session->request.set(boost::beast::http::field::user_agent, "mtxclient v0.1.0");
        session->request.set(boost::beast::http::field::content_type, "application/json");
        session->request.set(boost::beast::http::field::accept_encoding, "gzip,deflate");
        session->request.set(boost::beast::http::field::host, session->host);
        session->request.body() = serialize<Request>(req);
        session->request.prepare_payload();

        do_request(session);
}

// provides PUT functionality for the endpoints which dont respond with a body
template<class Request>
void
mtx::client::Client::put(const std::string &endpoint,
                         const Request &req,
                         std::function<void(RequestErr)> callback,
                         bool requires_auth)
{
        mtx::client::Client::put<Request, mtx::responses::Empty>(
          endpoint,
          req,
          [callback](const mtx::responses::Empty, RequestErr err) { callback(err); },
          requires_auth);
}

template<class Response>
void
mtx::client::Client::get(const std::string &endpoint,
                         std::function<void(const Response &, HeaderFields, RequestErr)> callback,
                         bool requires_auth)
{
        std::shared_ptr<Session> session = create_session<Response>(callback);

        if (!session)
                return;

        setup_auth(session, requires_auth);

        session->request.method(boost::beast::http::verb::get);
        session->request.target("/_matrix" + endpoint);
        session->request.set(boost::beast::http::field::user_agent, "mtxclient v0.1.0");
        session->request.set(boost::beast::http::field::accept_encoding, "gzip,deflate");
        session->request.set(boost::beast::http::field::host, session->host);
        session->request.prepare_payload();

        do_request(session);
}

template<class Response>
std::shared_ptr<mtx::client::Session>
mtx::client::Client::create_session(
  std::function<void(const Response &, HeaderFields, RequestErr)> callback)
{
        boost::asio::ssl::context ssl_ctx{boost::asio::ssl::context::sslv23_client};

        std::shared_ptr<Session> session = std::make_shared<Session>(
          ios_,
          ssl_ctx,
          server_,
          utils::random_token(),
          [callback, _this = shared_from_this()](
            RequestID,
            const boost::beast::http::response<boost::beast::http::string_body> &response,
            const boost::system::error_code &err_code) {
                  _this->ios_.post([callback, response, err_code]() {
                          Response response_data;
                          mtx::client::errors::ClientError client_error;

                          const auto header = response.base();

                          if (err_code) {
                                  client_error.error_code = err_code;
                                  return callback(response_data, header, client_error);
                          }

                          // Decompress the response.
                          const auto body = utils::decompress(
                            boost::iostreams::array_source{response.body().data(),
                                                           response.body().size()},
                            header["Content-Encoding"].to_string());

                          if (response.result() != boost::beast::http::status::ok) {
                                  client_error.status_code = response.result();

                                  // Try to parse the response in case we have an endpoint that
                                  // doesn't return an error struct for non 200 requests.
                                  try {
                                          response_data = deserialize<Response>(body);
                                  } catch (const nlohmann::json::exception &e) {
                                  }

                                  // The homeserver should return an error struct.
                                  try {
                                          nlohmann::json json_error       = json::parse(body);
                                          mtx::errors::Error matrix_error = json_error;

                                          client_error.matrix_error = matrix_error;
                                          return callback(response_data, header, client_error);
                                  } catch (const nlohmann::json::exception &e) {
                                          client_error.parse_error =
                                            std::string(e.what()) + ": " + body;

                                          return callback(response_data, header, client_error);
                                  }
                          }

                          // If we reach that point we most likely have a valid output from the
                          // homeserver.
                          try {
                                  callback(deserialize<Response>(body), header, {});
                          } catch (const nlohmann::json::exception &e) {
                                  client_error.parse_error = std::string(e.what()) + ": " + body;
                                  callback(response_data, header, client_error);
                          }
                  });
          },
          [callback](RequestID, const boost::system::error_code ec) {
                  Response response_data;

                  mtx::client::errors::ClientError client_error;
                  client_error.error_code = ec;

                  callback(response_data, {}, client_error);
          });

        // Set SNI Hostname (many hosts need this to handshake successfully)
        if (!SSL_set_tlsext_host_name(session->socket.native_handle(), server_.c_str())) {
                boost::system::error_code ec{static_cast<int>(::ERR_get_error()),
                                             boost::asio::error::get_ssl_category()};
                std::cerr << ec.message() << "\n";

                Response response_data;

                mtx::client::errors::ClientError client_error;
                client_error.error_code = ec;

                callback(response_data, {}, client_error);

                // Initialization failed.
                return nullptr;
        }

        return session;
}

template<class Payload, mtx::events::EventType Event>
void
mtx::client::Client::send_room_message(
  const mtx::identifiers::Room &room_id,
  const Payload &payload,
  std::function<void(const mtx::responses::EventId &, RequestErr)> callback)
{
        send_room_message<Payload, Event>(room_id, generate_txn_id(), payload, callback);
}

template<class Payload, mtx::events::EventType Event>
void
mtx::client::Client::send_room_message(
  const mtx::identifiers::Room &room_id,
  const std::string &txn_id,
  const Payload &payload,
  std::function<void(const mtx::responses::EventId &, RequestErr)> callback)
{
        const auto api_path = "/client/r0/rooms/" + room_id.to_string() + "/send/" +
                              mtx::events::to_string(Event) + "/" + txn_id;

        put<Payload, mtx::responses::EventId>(api_path, payload, callback);
}

template<class Payload, mtx::events::EventType Event>
void
mtx::client::Client::send_state_event(
  const mtx::identifiers::Room &room_id,
  const std::string &state_key,
  const Payload &payload,
  std::function<void(const mtx::responses::EventId &, RequestErr)> callback)
{
        const auto api_path = "/client/r0/rooms/" + room_id.to_string() + "/state/" +
                              mtx::events::to_string(Event) + "/" + state_key;

        put<Payload, mtx::responses::EventId>(api_path, payload, callback);
}

template<class Payload, mtx::events::EventType Event>
void
mtx::client::Client::send_state_event(
  const mtx::identifiers::Room &room_id,
  const Payload &payload,
  std::function<void(const mtx::responses::EventId &, RequestErr)> callback)
{
        send_state_event<Payload, Event>(room_id, "", payload, callback);
}
