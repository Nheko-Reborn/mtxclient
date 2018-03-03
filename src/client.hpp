#pragma once

#include <experimental/optional>
#include <memory>
#include <mutex>
#include <thread>

#include <boost/asio.hpp>
#include <boost/asio/ssl.hpp>
#include <boost/beast.hpp>
#include <boost/thread/thread.hpp>
#include <json.hpp>

#include "errors.hpp"
#include "mtx/requests.hpp"
#include "mtx/responses.hpp"
#include "session.hpp"
#include "utils.hpp"

namespace mtx {
namespace client {

//! The main object that the user will interact.
class Client : public std::enable_shared_from_this<Client>
{
public:
        Client(const std::string &server = "");

        //! Wait for the client to close.
        void close();
        //! Cancels the request.
        void cancel_request(RequestID request_id);
        //! Make a new request.
        void do_request(std::shared_ptr<Session> session);
        //! Return the number of pending requests.
        int active_sessions() const { return active_sessions_.size(); }
        //! Add an access token.
        void set_access_token(const std::string &token) { access_token_ = token; }
        //! Update the next batch token.
        void set_next_batch_token(const std::string &token) { next_batch_token_ = token; }
        //! Retrieve the current next batch token.
        std::string next_batch_token() const { return next_batch_token_; }

        using RequestErr = std::experimental::optional<mtx::client::errors::ClientError>;

        //! Perfom login.
        void login(const std::string &username,
                   const std::string &password,
                   std::function<void(const mtx::responses::Login &response, RequestErr err)>);
        //! Perform logout.
        void logout(std::function<void(const mtx::responses::Logout &response, RequestErr err)>);
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
        /* void invite_user(); */
        //! Perform sync.
        void sync(const std::string &filter,
                  const std::string &since,
                  bool full_state,
                  uint16_t timeout,
                  std::function<void(const mtx::responses::Sync &res, RequestErr err)>);
        //! Paginate through room messages.
        /* void get_messages(); */
        //! Send a message into a room.
        /* void send_room_message(); */
        //! Get the supported versions from the server.
        /* void versions(); */

        /* void download_room_avatar(); */
        /* void download_user_avatar(); */
        /* void download_media(); */

        /* void upload_image(); */
        /* void upload_file(); */
        /* void upload_audio(); */
        /* void upload_video(); */

        /* void upload_filter(); */

        /* void send_typing_notification(); */
        /* void remove_typing_notification(); */
        /* void read_event(); */

private:
        template<class Request, class Response>
        void post(
          const std::string &endpoint,
          const Request &req,
          std::function<void(const Response &,
                             std::experimental::optional<mtx::client::errors::ClientError>)>,
          bool requires_auth = true);

        template<class Response>
        void get(const std::string &endpoint,
                 std::function<void(const Response &,
                                    std::experimental::optional<mtx::client::errors::ClientError>)>,
                 bool requires_auth = true);

        template<class Response, class Callback>
        std::shared_ptr<Session> create_session(const Callback &callback);

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

        //! Keeps tracks for the active sessions.
        std::map<RequestID, std::shared_ptr<Session>> active_sessions_;
        //! Used to synchronize access to `active_sessions_`.
        std::mutex active_sessions_guard_;
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
        //! The token that will be used as the 'since' parameter on the next sync request.
        std::string next_batch_token_;
};
}
}

template<class Request, class Response>
void
mtx::client::Client::post(
  const std::string &endpoint,
  const Request &req,
  std::function<void(const Response &,
                     std::experimental::optional<mtx::client::errors::ClientError>)> callback,
  bool requires_auth)
{
        // Serialize request.
        nlohmann::json j = req;

        using CallbackType = std::function<void(
          const Response &, std::experimental::optional<mtx::client::errors::ClientError>)>;

        std::shared_ptr<Session> session = create_session<Response, CallbackType>(callback);

        session->request.method(boost::beast::http::verb::post);
        session->request.target("/_matrix/client/r0" + endpoint);
        session->request.set(boost::beast::http::field::user_agent, "mtxclient v0.1.0");
        session->request.set(boost::beast::http::field::content_type, "application/json");
        session->request.set(boost::beast::http::field::host, session->host);
        if (requires_auth && !access_token_.empty())
                session->request.set(boost::beast::http::field::authorization,
                                     "Bearer " + access_token_);
        session->request.body() = j.dump();
        session->request.prepare_payload();

        do_request(session);
}

template<class Response>
void
mtx::client::Client::get(
  const std::string &endpoint,
  std::function<void(const Response &,
                     std::experimental::optional<mtx::client::errors::ClientError>)> callback,
  bool requires_auth)
{
        using CallbackType = std::function<void(
          const Response &, std::experimental::optional<mtx::client::errors::ClientError>)>;

        std::shared_ptr<Session> session = create_session<Response, CallbackType>(callback);

        session->request.method(boost::beast::http::verb::get);
        session->request.target("/_matrix/client/r0" + endpoint);
        session->request.set(boost::beast::http::field::user_agent, "mtxclient v0.1.0");
        session->request.set(boost::beast::http::field::host, session->host);
        if (requires_auth && !access_token_.empty())
                session->request.set(boost::beast::http::field::authorization,
                                     "Bearer " + access_token_);
        session->request.prepare_payload();

        do_request(session);
}

template<class Response, class Callback>
std::shared_ptr<mtx::client::Session>
mtx::client::Client::create_session(const Callback &callback)
{
        boost::asio::ssl::context ssl_ctx{boost::asio::ssl::context::sslv23_client};

        std::shared_ptr<Session> session = std::make_shared<Session>(
          ios_,
          ssl_ctx,
          server_,
          utils::random_token(),
          [callback,
           this](RequestID,
                 const boost::beast::http::response<boost::beast::http::string_body> &response,
                 const boost::system::error_code &err_code) {

                  ios_.post([callback, response, err_code]() {
                          Response response_data;
                          mtx::client::errors::ClientError client_error;

                          if (err_code) {
                                  client_error.error_code = err_code;
                                  return callback(response_data, client_error);
                          }

                          // TODO: handle http error.
                          if (response.result() != boost::beast::http::status::ok) {
                                  // TODO: handle unknown error.
                                  client_error.status_code = response.result();

                                  try {
                                          nlohmann::json json_error = json::parse(response.body());
                                          mtx::errors::Error matrix_error = json_error;

                                          client_error.matrix_error = matrix_error;
                                          return callback(response_data, client_error);
                                  } catch (nlohmann::json::exception &e) {
                                          std::cout << e.what() << ": Couldn't parse response\n"
                                                    << response.body().data() << std::endl;
                                  }
                          }

                          try {
                                  nlohmann::json json_data = json::parse(response.body().data());
                                  response_data            = json_data;
                          } catch (nlohmann::json::exception &e) {
                                  std::cout << e.what() << ": Couldn't parse response\n"
                                            << response.body().data() << std::endl;
                          }

                          callback(response_data, {});
                  });
          },
          [callback](RequestID, const boost::system::error_code ec) {
                  Response response_data;

                  mtx::client::errors::ClientError client_error;
                  client_error.error_code = ec;

                  callback(response_data, client_error);
          });

        // Set SNI Hostname (many hosts need this to handshake successfully)
        // TODO: handle the error
        if (!SSL_set_tlsext_host_name(session->socket.native_handle(), server_.c_str())) {
                boost::system::error_code ec{static_cast<int>(::ERR_get_error()),
                                             boost::asio::error::get_ssl_category()};
                std::cerr << ec.message() << "\n";
                return session;
        }

        return session;
}
