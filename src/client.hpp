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

using RequestErr   = const boost::optional<mtx::client::errors::ClientError> &;
using HeaderFields = const boost::optional<boost::beast::http::fields> &;
using ErrCallback  = std::function<void(RequestErr)>;

template<class Response>
using Callback = std::function<void(const Response &, RequestErr)>;

template<class Response>
using HeadersCallback = std::function<void(const Response &, HeaderFields, RequestErr)>;

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

        //! Perfom login.
        void login(const std::string &username,
                   const std::string &password,
                   Callback<mtx::responses::Login> cb);

        //! Register by not expecting a registration flow.
        void registration(const std::string &user,
                          const std::string &pass,
                          Callback<mtx::responses::Register> cb);

        //! Register through a registration flow.
        void flow_register(const std::string &user,
                           const std::string &pass,
                           Callback<mtx::responses::RegistrationFlows> cb);

        //! Complete the flow registration.
        void flow_response(const std::string &user,
                           const std::string &pass,
                           const std::string &session,
                           const std::string &flow_type,
                           Callback<mtx::responses::Register> cb);

        //! Perform logout.
        void logout(Callback<mtx::responses::Logout> cb);
        //! Change avatar.
        void set_avatar_url(const std::string &avatar_url, ErrCallback cb);
        //! Change displayname.
        void set_displayname(const std::string &displayname, ErrCallback cb);
        //! Get user profile.
        void get_profile(const mtx::identifiers::User &user_id,
                         Callback<mtx::responses::Profile> cb);
        //! Get user avatar URL.
        void get_avatar_url(const mtx::identifiers::User &user_id,
                            Callback<mtx::responses::AvatarUrl> cb);
        //! Create a room with the given options.
        void create_room(const mtx::requests::CreateRoom &room_options,
                         Callback<mtx::responses::CreateRoom> cb);
        //! Join a room by its room_id.
        void join_room(const mtx::identifiers::Room &room_id, Callback<nlohmann::json> cb);
        //! Join a room by an alias or a room_id.
        void join_room(const std::string &room, Callback<nlohmann::json> cb);
        //! Leave a room by its room_id.
        void leave_room(const mtx::identifiers::Room &room_id, Callback<nlohmann::json> cb);
        //! Invite a user to a room.
        void invite_user(const mtx::identifiers::Room &room_id,
                         const std::string &user_id,
                         Callback<mtx::responses::RoomInvite> cb);
        //! Perform sync.
        void sync(const std::string &filter,
                  const std::string &since,
                  bool full_state,
                  uint16_t timeout,
                  Callback<nlohmann::json> cb);

        //! Paginate through room messages.
        void messages(const mtx::identifiers::Room &room_id,
                      const std::string &from,
                      const std::string &to,
                      PaginationDirection dir,
                      uint16_t limit,
                      const std::string &filter,
                      Callback<mtx::responses::Messages> cb);

        //! Get the supported versions from the server.
        void versions(Callback<mtx::responses::Versions> cb);

        //! Mark an event as read.
        void read_event(const mtx::identifiers::Room &room_id,
                        const mtx::identifiers::Event &event_id,
                        ErrCallback cb);

        //! Upload a filter
        void upload_filter(const nlohmann::json &j, Callback<mtx::responses::FilterId> cb);

        //! Upload data to the content repository.
        void upload(const std::string &data,
                    const std::string &content_type,
                    const std::string &filename,
                    Callback<mtx::responses::ContentURI> cb);
        //! Retrieve data from the content repository.
        void download(const std::string &server,
                      const std::string &media_id,
                      std::function<void(const std::string &data,
                                         const std::string &content_type,
                                         const std::string &original_filename,
                                         RequestErr err)> cb);
        //! Send typing notifications to the room.
        void start_typing(const mtx::identifiers::Room &room_id, uint64_t timeout, ErrCallback cb);
        //! Remove typing notifications from the room.
        void stop_typing(const mtx::identifiers::Room &room_id, ErrCallback cb);
        //! Send a room message with auto-generated transaction id.
        template<class Payload, mtx::events::EventType Event>
        void send_room_message(const mtx::identifiers::Room &room_id,
                               const Payload &payload,
                               Callback<mtx::responses::EventId> cb);
        //! Send a room message by providing transaction id.
        template<class Payload, mtx::events::EventType Event>
        void send_room_message(const mtx::identifiers::Room &room_id,
                               const std::string &txn_id,
                               const Payload &payload,
                               Callback<mtx::responses::EventId> cb);
        //! Send a state event by providing the state key.
        template<class Payload, mtx::events::EventType Event>
        void send_state_event(const mtx::identifiers::Room &room_id,
                              const std::string &state_key,
                              const Payload &payload,
                              Callback<mtx::responses::EventId> cb);
        //! Send a state event with an empty state key.
        template<class Payload, mtx::events::EventType Event>
        void send_state_event(const mtx::identifiers::Room &room_id,
                              const Payload &payload,
                              Callback<mtx::responses::EventId> cb);

        //! Send send-to-device events to a set of client devices with a specified transaction id.
        void send_to_device(const std::string &event_type,
                            const std::string &txid,
                            const nlohmann::json &body,
                            ErrCallback cb);

        //! Send send-to-device events to a set of client devices with a generated transaction id.
        void send_to_device(const std::string &event_type,
                            const nlohmann::json &body,
                            ErrCallback cb)
        {
                send_to_device(event_type, generate_txn_id(), body, cb);
        }

        //
        // Encryption related endpoints.
        //

        //! Upload identity keys & one time keys.
        void upload_keys(const mtx::requests::UploadKeys &req,
                         Callback<mtx::responses::UploadKeys> cb);

        //! Returns the current devices and identity keys for the given users.
        void query_keys(const mtx::requests::QueryKeys &req,
                        Callback<mtx::responses::QueryKeys> cb);

        //! Claims one-time keys for use in pre-key messages.
        void claim_keys(const mtx::identifiers::User &user,
                        const std::vector<std::string> &devices,
                        Callback<mtx::responses::ClaimKeys> cb);

        //! Gets a list of users who have updated their device identity keys
        //! since a previous sync token.
        void key_changes(const std::string &from,
                         const std::string &to,
                         Callback<mtx::responses::KeyChanges> cb);

        //! Enable encryption in a room by sending a `m.room.encryption` state event.
        void enable_encryption(const mtx::identifiers::Room &room,
                               Callback<mtx::responses::EventId> cb);

private:
        template<class Request, class Response>
        void post(const std::string &endpoint,
                  const Request &req,
                  Callback<Response> cb,
                  bool requires_auth              = true,
                  const std::string &content_type = "application/json");

        // put function for the PUT HTTP requests that send responses
        template<class Request, class Response>
        void put(const std::string &endpoint,
                 const Request &req,
                 Callback<Response> cb,
                 bool requires_auth = true);

        template<class Request>
        void put(const std::string &endpoint,
                 const Request &req,
                 ErrCallback cb,
                 bool requires_auth = true);

        template<class Response>
        void get(const std::string &endpoint,
                 HeadersCallback<Response> cb,
                 bool requires_auth = true);

        template<class Response>
        std::shared_ptr<Session> create_session(HeadersCallback<Response> callback);

        //! Setup http header with the access token if needed.
        void setup_auth(Session *session, bool auth);

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
        boost::optional<boost::asio::io_service::work> work_;
        //! Worker threads for the requests.
        boost::thread_group thread_group_;
        //! Used to resolve DNS names.
        boost::asio::ip::tcp::resolver resolver_;
        //! SSL context for requests.
        boost::asio::ssl::context ssl_ctx_{boost::asio::ssl::context::sslv23_client};
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

template<class Request, class Response>
void
mtx::client::Client::post(const std::string &endpoint,
                          const Request &req,
                          Callback<Response> callback,
                          bool requires_auth,
                          const std::string &content_type)
{
        auto session = create_session<Response>(
          [callback](const Response &res, HeaderFields, RequestErr err) { callback(res, err); });

        if (!session)
                return;

        setup_auth(session.get(), requires_auth);
        setup_headers<Request, boost::beast::http::verb::post>(
          session.get(), req, endpoint, content_type);

        do_request(std::move(session));
}

// put function for the PUT HTTP requests that send responses
template<class Request, class Response>
void
mtx::client::Client::put(const std::string &endpoint,
                         const Request &req,
                         Callback<Response> callback,
                         bool requires_auth)
{
        auto session = create_session<Response>(
          [callback](const Response &res, HeaderFields, RequestErr err) { callback(res, err); });

        if (!session)
                return;

        setup_auth(session.get(), requires_auth);
        setup_headers<Request, boost::beast::http::verb::put>(
          session.get(), req, endpoint, "application/json");

        do_request(std::move(session));
}

// provides PUT functionality for the endpoints which dont respond with a body
template<class Request>
void
mtx::client::Client::put(const std::string &endpoint,
                         const Request &req,
                         ErrCallback callback,
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
                         HeadersCallback<Response> callback,
                         bool requires_auth)
{
        auto session = create_session<Response>(callback);

        if (!session)
                return;

        setup_auth(session.get(), requires_auth);
        setup_headers<std::string, boost::beast::http::verb::get>(session.get(), {}, endpoint);

        do_request(std::move(session));
}

template<class Response>
std::shared_ptr<mtx::client::Session>
mtx::client::Client::create_session(HeadersCallback<Response> callback)
{
        auto session = std::make_shared<Session>(
          ios_,
          ssl_ctx_,
          server_,
          utils::random_token(),
          [callback](RequestID,
                     const boost::beast::http::response<boost::beast::http::string_body> &response,
                     const boost::system::error_code &err_code) {
                  Response response_data;
                  mtx::client::errors::ClientError client_error;

                  const auto header = response.base();

                  if (err_code) {
                          client_error.error_code = err_code;
                          return callback(response_data, header, client_error);
                  }

                  // Decompress the response.
                  const auto body = utils::decompress(
                    boost::iostreams::array_source{response.body().data(), response.body().size()},
                    header["Content-Encoding"].to_string());

                  if (response.result() != boost::beast::http::status::ok) {
                          client_error.status_code = response.result();

                          // Try to parse the response in case we have an endpoint that
                          // doesn't return an error struct for non 200 requests.
                          try {
                                  response_data = utils::deserialize<Response>(body);
                          } catch (const nlohmann::json::exception &e) {
                          }

                          // The homeserver should return an error struct.
                          try {
                                  nlohmann::json json_error       = json::parse(body);
                                  mtx::errors::Error matrix_error = json_error;

                                  client_error.matrix_error = matrix_error;
                                  return callback(response_data, header, client_error);
                          } catch (const nlohmann::json::exception &e) {
                                  client_error.parse_error = std::string(e.what()) + ": " + body;

                                  return callback(response_data, header, client_error);
                          }
                  }

                  // If we reach that point we most likely have a valid output from the
                  // homeserver.
                  try {
                          callback(utils::deserialize<Response>(body), header, {});
                  } catch (const nlohmann::json::exception &e) {
                          client_error.parse_error = std::string(e.what()) + ": " + body;
                          callback(response_data, header, client_error);
                  }
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

        return std::move(session);
}

template<class Payload, mtx::events::EventType Event>
void
mtx::client::Client::send_room_message(const mtx::identifiers::Room &room_id,
                                       const Payload &payload,
                                       Callback<mtx::responses::EventId> callback)
{
        send_room_message<Payload, Event>(room_id, generate_txn_id(), payload, callback);
}

template<class Payload, mtx::events::EventType Event>
void
mtx::client::Client::send_room_message(const mtx::identifiers::Room &room_id,
                                       const std::string &txn_id,
                                       const Payload &payload,
                                       Callback<mtx::responses::EventId> callback)
{
        const auto api_path = "/client/r0/rooms/" + room_id.to_string() + "/send/" +
                              mtx::events::to_string(Event) + "/" + txn_id;

        put<Payload, mtx::responses::EventId>(api_path, payload, callback);
}

template<class Payload, mtx::events::EventType Event>
void
mtx::client::Client::send_state_event(const mtx::identifiers::Room &room_id,
                                      const std::string &state_key,
                                      const Payload &payload,
                                      Callback<mtx::responses::EventId> callback)
{
        const auto api_path = "/client/r0/rooms/" + room_id.to_string() + "/state/" +
                              mtx::events::to_string(Event) + "/" + state_key;

        put<Payload, mtx::responses::EventId>(api_path, payload, callback);
}

template<class Payload, mtx::events::EventType Event>
void
mtx::client::Client::send_state_event(const mtx::identifiers::Room &room_id,
                                      const Payload &payload,
                                      Callback<mtx::responses::EventId> callback)
{
        send_state_event<Payload, Event>(room_id, "", payload, callback);
}
