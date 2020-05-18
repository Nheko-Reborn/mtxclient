#pragma once

#if __has_include(<nlohmann/json_fwd.hpp>)
#include <nlohmann/json_fwd.hpp>
#else
#include <nlohmann/json.hpp>
#endif

#include "mtx/errors.hpp"             // for Error
#include "mtx/events.hpp"             // for EventType, to_string, json
#include "mtx/events/collections.hpp" // for TimelineEvents
#include "mtx/identifiers.hpp"        // for User
#include "mtx/identifiers.hpp"        // for Class user
#include "mtx/pushrules.hpp"
#include "mtx/requests.hpp"
#include "mtx/responses/empty.hpp"   // for Empty, Logout, RoomInvite
#include "mtxclient/http/errors.hpp" // for ClientError
#include "mtxclient/utils.hpp"       // for random_token, url_encode, des...

#include <boost/beast/http/fields.hpp> // for fields
#include <boost/beast/http/status.hpp> // for status
#include <boost/system/error_code.hpp> // for error_code

#include <cstdint>    // for uint16_t, uint64_t
#include <functional> // for function
#include <memory>     // for allocator, shared_ptr, enable...
#include <optional>   // for optional
#include <string>     // for string, operator+, char_traits
#include <utility>    // for move
#include <vector>     // for vector

// forward declarations
namespace mtx {
namespace http {
struct ClientPrivate;
struct Session;
}
namespace requests {
struct CreateRoom;
struct Login;
struct QueryKeys;
struct UploadKeys;
}
namespace responses {
struct AvatarUrl;
struct ClaimKeys;
struct ContentURI;
struct CreateRoom;
struct EventId;
struct FilterId;
struct GroupId;
struct GroupProfile;
struct JoinedGroups;
struct KeyChanges;
struct Login;
struct LoginFlows;
struct Messages;
struct Notifications;
struct Profile;
struct QueryKeys;
struct Register;
struct Sync;
struct UploadKeys;
struct Versions;
struct WellKnown;
}
}

namespace mtx {
namespace http {

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

using RequestErr   = const std::optional<mtx::http::ClientError> &;
using HeaderFields = const std::optional<boost::beast::http::fields> &;
using ErrCallback  = std::function<void(RequestErr)>;

template<class Response>
using Callback = std::function<void(const Response &, RequestErr)>;

template<class Response>
using HeadersCallback    = std::function<void(const Response &, HeaderFields, RequestErr)>;
using TypeErasedCallback = std::function<void(HeaderFields,
                                              const std::string &,
                                              const boost::system::error_code &,
                                              boost::beast::http::status)>;

//! Sync configuration options.
struct SyncOpts
{
        //! Filter to apply.
        std::string filter;
        //! Sync pagination token.
        std::string since;
        //! The amount of msecs to wait for long polling.
        uint16_t timeout = 30'000;
        //! Wheter to include the full state of each room.
        bool full_state = false;
};

//! Configuration for the /messages endpoint.
struct MessagesOpts
{
        std::string room_id;
        std::string from;
        std::string to;
        std::string filter;

        PaginationDirection dir = PaginationDirection::Backwards;

        uint16_t limit = 30;
};

//! Configuration for thumbnail retrieving.
struct ThumbOpts
{
        //! The desired width of the thumbnail.
        uint16_t width = 128;
        //! The desired height of the thumbnail.
        uint16_t height = 128;
        //! The desired resizing method. One of: ["crop", "scale"]
        std::string method = "crop";
        //! A mxc URI which points to the content.
        std::string mxc_url;
};

struct ClientPrivate;
struct Session;

//! The main object that the user will interact.
class Client : public std::enable_shared_from_this<Client>
{
public:
        Client(const std::string &server = "", uint16_t port = 443);
        ~Client();

        //! Wait for the client to close.
        void close(bool force = false);
        //! Set the homeserver domain name.
        void set_user(const mtx::identifiers::User &user) { user_id_ = user; }
        //! Set the device ID.
        void set_device_id(const std::string &device_id) { device_id_ = device_id; }
        //! Set the homeserver domain name.
        void set_server(const std::string &server);
        //! Retrieve the homeserver domain name.
        std::string server() { return server_; };
        //! Set the homeserver port.
        void set_port(uint16_t port) { port_ = port; };
        //! Retrieve the homeserver port.
        uint16_t port() { return port_; };
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
        std::string generate_txn_id() { return client::utils::random_token(32, false); }
        //! Abort all active pending requests.
        void shutdown();
        //! Remove all saved configuration.
        void clear()
        {
                device_id_.clear();
                access_token_.clear();
                next_batch_token_.clear();
                server_.clear();
                port_ = 443;
        }

        //! Perfom login.
        void login(const std::string &username,
                   const std::string &password,
                   Callback<mtx::responses::Login> cb);
        void login(const std::string &username,
                   const std::string &password,
                   const std::string &device_name,
                   Callback<mtx::responses::Login> cb);
        void login(const mtx::requests::Login &req, Callback<mtx::responses::Login> cb);

        //! Get the supported login flows
        void get_login(Callback<mtx::responses::LoginFlows> cb);
        //! Get url to navigate to for sso login flow
        //! Open this in a browser
        std::string login_sso_redirect(std::string redirectUrl);
        //! Lookup real server to connect to.
        //! Call set_server with the returned homeserver url after this
        void well_known(Callback<mtx::responses::WellKnown> cb);

        //! Register
        //! If this fails with 401, continue with the flows returned in the error struct
        void registration(const std::string &user,
                          const std::string &pass,
                          Callback<mtx::responses::Register> cb);

        //! Register and additionally provide an auth dict. This needs to be called, if the initial
        //! register failed with 401
        void registration(const std::string &user,
                          const std::string &pass,
                          const user_interactive::Auth &auth,
                          Callback<mtx::responses::Register> cb);

        //! Paginate through the list of events that the user has been,
        //! or would have been notified about.
        void notifications(uint64_t limit,
                           const std::string &from,
                           const std::string &only,
                           Callback<mtx::responses::Notifications> cb);

        //! Retrieve all push rulesets for this user.
        void get_pushrules(Callback<pushrules::GlobalRuleset> cb);

        //! Retrieve a single specified push rule.
        void get_pushrules(const std::string &scope,
                           const std::string &kind,
                           const std::string &ruleId,
                           Callback<pushrules::PushRule> cb);

        //! This endpoint removes the push rule defined in the path.
        void delete_pushrules(const std::string &scope,
                              const std::string &kind,
                              const std::string &ruleId,
                              ErrCallback cb);

        //! This endpoint allows the creation, modification and deletion of pushers for this user
        //! ID.
        void put_pushrules(const std::string &scope,
                           const std::string &kind,
                           const std::string &ruleId,
                           const pushrules::PushRule &rule,
                           ErrCallback cb,
                           const std::string &before = "",
                           const std::string &after  = "");

        //! Retrieve a single specified push rule.
        void get_pushrules_enabled(const std::string &scope,
                                   const std::string &kind,
                                   const std::string &ruleId,
                                   Callback<pushrules::Enabled> cb);

        //! This endpoint allows clients to enable or disable the specified push rule.
        void put_pushrules_enabled(const std::string &scope,
                                   const std::string &kind,
                                   const std::string &ruleId,
                                   bool enabled,
                                   ErrCallback cb);

        //! This endpoint get the actions for the specified push rule.
        void get_pushrules_actions(const std::string &scope,
                                   const std::string &kind,
                                   const std::string &ruleId,
                                   Callback<pushrules::actions::Actions> cb);

        //! This endpoint allows clients to change the actions of a push rule. This can be used to
        //! change the actions of builtin rules.
        void put_pushrules_actions(const std::string &scope,
                                   const std::string &kind,
                                   const std::string &ruleId,
                                   const pushrules::actions::Actions &actions,
                                   ErrCallback cb);

        //! Perform logout.
        void logout(Callback<mtx::responses::Logout> cb);
        //! Change avatar.
        void set_avatar_url(const std::string &avatar_url, ErrCallback cb);
        //! Change displayname.
        void set_displayname(const std::string &displayname, ErrCallback cb);
        //! Get user profile.
        void get_profile(const std::string &user_id, Callback<mtx::responses::Profile> cb);
        //! Get user avatar URL.
        void get_avatar_url(const std::string &user_id, Callback<mtx::responses::AvatarUrl> cb);

        //! List the tags set by a user on a room.
        void get_tags(const std::string &room_id, Callback<mtx::events::account_data::Tags> cb);
        //! Add a tag to the room.
        void put_tag(const std::string &room_id,
                     const std::string &tag_name,
                     const mtx::events::account_data::Tag &tag,
                     ErrCallback cb);
        //! Remove a tag from the room.
        void delete_tag(const std::string &room_id, const std::string &tag_name, ErrCallback cb);

        //! Create a room with the given options.
        void create_room(const mtx::requests::CreateRoom &room_options,
                         Callback<mtx::responses::CreateRoom> cb);
        //! Join a room by an alias or a room_id.
        void join_room(const std::string &room, Callback<nlohmann::json> cb);
        //! Leave a room by its room_id.
        void leave_room(const std::string &room_id, Callback<nlohmann::json> cb);
        //! Invite a user to a room.
        void invite_user(const std::string &room_id,
                         const std::string &user_id,
                         Callback<mtx::responses::RoomInvite> cb,
                         const std::string &reason = "");
        //! Kick a user from a room.
        void kick_user(const std::string &room_id,
                       const std::string &user_id,
                       Callback<mtx::responses::Empty> cb,
                       const std::string &reason = "");
        //! Ban a user from a room.
        void ban_user(const std::string &room_id,
                      const std::string &user_id,
                      Callback<mtx::responses::Empty> cb,
                      const std::string &reason = "");
        //! Unban a user from a room.
        void unban_user(const std::string &room_id,
                        const std::string &user_id,
                        Callback<mtx::responses::Empty> cb,
                        const std::string &reason = "");

        //! Perform sync.
        void sync(const SyncOpts &opts, Callback<mtx::responses::Sync> cb);

        //! Paginate through room messages.
        void messages(const MessagesOpts &opts, Callback<mtx::responses::Messages> cb);

        //! Get the supported versions from the server.
        void versions(Callback<mtx::responses::Versions> cb);

        //! Mark an event as read.
        void read_event(const std::string &room_id, const std::string &event_id, ErrCallback cb);

        //! Redact an event from a room.
        void redact_event(const std::string &room_id,
                          const std::string &event_id,
                          Callback<mtx::responses::EventId> cb);

        //! Upload a filter
        void upload_filter(const nlohmann::json &j, Callback<mtx::responses::FilterId> cb);

        //! Upload data to the content repository.
        void upload(const std::string &data,
                    const std::string &content_type,
                    const std::string &filename,
                    Callback<mtx::responses::ContentURI> cb);
        //! Retrieve data from the content repository.
        void download(const std::string &mxc_url,
                      std::function<void(const std::string &data,
                                         const std::string &content_type,
                                         const std::string &original_filename,
                                         RequestErr err)> cb);
        void download(const std::string &server,
                      const std::string &media_id,
                      std::function<void(const std::string &data,
                                         const std::string &content_type,
                                         const std::string &original_filename,
                                         RequestErr err)> cb);

        //! Retrieve a thumbnail from the given mxc url.
        //! If the thumbnail isn't found and `try_download` is `true` it will try
        //! to use the `/download` endpoint to retrieve the media.
        void get_thumbnail(const ThumbOpts &opts,
                           Callback<std::string> cb,
                           bool try_download = true);

        //! Send typing notifications to the room.
        void start_typing(const std::string &room_id, uint64_t timeout, ErrCallback cb);
        //! Remove typing notifications from the room.
        void stop_typing(const std::string &room_id, ErrCallback cb);
        //! Get a single event.
        void get_event(const std::string &room_id,
                       const std::string &event_id,
                       Callback<mtx::events::collections::TimelineEvents> cb);
        //! Send a room message with auto-generated transaction id.
        template<class Payload, mtx::events::EventType Event>
        void send_room_message(const std::string &room_id,
                               const Payload &payload,
                               Callback<mtx::responses::EventId> cb);
        //! Send a room message by providing transaction id.
        template<class Payload, mtx::events::EventType Event>
        void send_room_message(const std::string &room_id,
                               const std::string &txn_id,
                               const Payload &payload,
                               Callback<mtx::responses::EventId> cb);
        //! Send a state event by providing the state key.
        template<class Payload, mtx::events::EventType Event>
        void send_state_event(const std::string &room_id,
                              const std::string &state_key,
                              const Payload &payload,
                              Callback<mtx::responses::EventId> cb);
        //! Send a state event with an empty state key.
        template<class Payload, mtx::events::EventType Event>
        void send_state_event(const std::string &room_id,
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
        //! Send send-to-device events to a set of client devices with a specified transaction id.
        template<typename EventContent, mtx::events::EventType Event>
        void send_to_device(
          const std::string &txid,
          const std::map<mtx::identifiers::User, std::map<std::string, EventContent>> &messages,
          ErrCallback callback)
        {
                json j;
                for (const auto &[user, deviceToMessage] : messages)
                        for (const auto &[deviceid, message] : deviceToMessage)
                                j["messages"][user.to_string()][deviceid] = message;

                send_to_device(mtx::events::to_string(Event), txid, j, callback);
        }

        //
        // Group related endpoints.
        //

        void create_group(const std::string &localpart, Callback<mtx::responses::GroupId> cb);
        void joined_groups(Callback<mtx::responses::JoinedGroups> cb);
        void group_profile(const std::string &group_id, Callback<mtx::responses::GroupProfile> cb);
        void group_rooms(const std::string &group_id, Callback<nlohmann::json> cb);
        void set_group_profile(const std::string &group_id,
                               nlohmann::json &req,
                               Callback<nlohmann::json> cb);
        void add_room_to_group(const std::string &room_id,
                               const std::string &group_id,
                               ErrCallback cb);

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
        void claim_keys(const std::string &user,
                        const std::vector<std::string> &devices,
                        Callback<mtx::responses::ClaimKeys> cb);

        //! Gets a list of users who have updated their device identity keys
        //! since a previous sync token.
        void key_changes(const std::string &from,
                         const std::string &to,
                         Callback<mtx::responses::KeyChanges> cb);

        //! Enable encryption in a room by sending a `m.room.encryption` state event.
        void enable_encryption(const std::string &room, Callback<mtx::responses::EventId> cb);

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
                 bool requires_auth                    = true,
                 const std::string &endpoint_namespace = "/_matrix");

        // type erased versions of http verbs
        void post(const std::string &endpoint,
                  const json &req,
                  TypeErasedCallback cb,
                  bool requires_auth,
                  const std::string &content_type);

        void put(const std::string &endpoint,
                 const json &req,
                 TypeErasedCallback cb,
                 bool requires_auth);

        void get(const std::string &endpoint,
                 TypeErasedCallback cb,
                 bool requires_auth,
                 const std::string &endpoint_namespace);

        void delete_(const std::string &endpoint, ErrCallback cb, bool requires_auth = true);

        template<class Response>
        TypeErasedCallback prepare_callback(HeadersCallback<Response> callback);

        std::shared_ptr<Session> create_session(TypeErasedCallback type_erased_cb);

        //! Setup http header with the access token if needed.
        void setup_auth(Session *session, bool auth);

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

        std::unique_ptr<ClientPrivate> p;
};
}
}

template<class Request, class Response>
void
mtx::http::Client::post(const std::string &endpoint,
                        const Request &req,
                        Callback<Response> callback,
                        bool requires_auth,
                        const std::string &content_type)
{
        post(
          endpoint,
          client::utils::serialize(req),
          prepare_callback<Response>(
            [callback](const Response &res, HeaderFields, RequestErr err) { callback(res, err); }),
          requires_auth,
          content_type);
}

// put function for the PUT HTTP requests that send responses
template<class Request, class Response>
void
mtx::http::Client::put(const std::string &endpoint,
                       const Request &req,
                       Callback<Response> callback,
                       bool requires_auth)
{
        put(
          endpoint,
          client::utils::serialize(req),
          prepare_callback<Response>(
            [callback](const Response &res, HeaderFields, RequestErr err) { callback(res, err); }),
          requires_auth);
}

// provides PUT functionality for the endpoints which dont respond with a body
template<class Request>
void
mtx::http::Client::put(const std::string &endpoint,
                       const Request &req,
                       ErrCallback callback,
                       bool requires_auth)
{
        mtx::http::Client::put<Request, mtx::responses::Empty>(
          endpoint,
          req,
          [callback](const mtx::responses::Empty, RequestErr err) { callback(err); },
          requires_auth);
}

template<class Response>
void
mtx::http::Client::get(const std::string &endpoint,
                       HeadersCallback<Response> callback,
                       bool requires_auth,
                       const std::string &endpoint_namespace)
{
        get(endpoint, prepare_callback<Response>(callback), requires_auth, endpoint_namespace);
}

template<class Response>
mtx::http::TypeErasedCallback
mtx::http::Client::prepare_callback(HeadersCallback<Response> callback)
{
        auto type_erased_cb = [callback](HeaderFields headers,
                                         const std::string &body,
                                         const boost::system::error_code &err_code,
                                         boost::beast::http::status status_code) {
                Response response_data;
                mtx::http::ClientError client_error;

                if (err_code) {
                        client_error.error_code = err_code;
                        return callback(response_data, headers, client_error);
                }

                // We only count 2xx status codes as success.
                if (static_cast<int>(status_code) < 200 || static_cast<int>(status_code) >= 300) {
                        client_error.status_code = status_code;

                        // Try to parse the response in case we have an endpoint that
                        // doesn't return an error struct for non 200 requests.
                        try {
                                response_data = client::utils::deserialize<Response>(body);
                        } catch (const nlohmann::json::exception &e) {
                        }

                        // The homeserver should return an error struct.
                        try {
                                nlohmann::json json_error       = json::parse(body);
                                mtx::errors::Error matrix_error = json_error;

                                client_error.matrix_error = matrix_error;
                                return callback(response_data, headers, client_error);
                        } catch (const nlohmann::json::exception &e) {
                                client_error.parse_error = std::string(e.what()) + ": " + body;

                                return callback(response_data, headers, client_error);
                        }
                }

                // If we reach that point we most likely have a valid output from the
                // homeserver.
                try {
                        auto res = client::utils::deserialize<Response>(body);
                        callback(std::move(res), headers, {});
                } catch (const nlohmann::json::exception &e) {
                        client_error.parse_error = std::string(e.what()) + ": " + body;
                        callback(response_data, headers, client_error);
                }
        };

        return type_erased_cb;
}

template<class Payload, mtx::events::EventType Event>
void
mtx::http::Client::send_room_message(const std::string &room_id,
                                     const Payload &payload,
                                     Callback<mtx::responses::EventId> callback)
{
        send_room_message<Payload, Event>(room_id, generate_txn_id(), payload, callback);
}

template<class Payload, mtx::events::EventType Event>
void
mtx::http::Client::send_room_message(const std::string &room_id,
                                     const std::string &txn_id,
                                     const Payload &payload,
                                     Callback<mtx::responses::EventId> callback)
{
        const auto api_path = "/client/r0/rooms/" + room_id + "/send/" +
                              mtx::events::to_string(Event) + "/" +
                              mtx::client::utils::url_encode(txn_id);

        put<Payload, mtx::responses::EventId>(api_path, payload, callback);
}

template<class Payload, mtx::events::EventType Event>
void
mtx::http::Client::send_state_event(const std::string &room_id,
                                    const std::string &state_key,
                                    const Payload &payload,
                                    Callback<mtx::responses::EventId> callback)
{
        const auto api_path = "/client/r0/rooms/" + mtx::client::utils::url_encode(room_id) +
                              "/state/" + mtx::events::to_string(Event) + "/" +
                              mtx::client::utils::url_encode(state_key);

        put<Payload, mtx::responses::EventId>(api_path, payload, callback);
}

template<class Payload, mtx::events::EventType Event>
void
mtx::http::Client::send_state_event(const std::string &room_id,
                                    const Payload &payload,
                                    Callback<mtx::responses::EventId> callback)
{
        send_state_event<Payload, Event>(room_id, "", payload, callback);
}
