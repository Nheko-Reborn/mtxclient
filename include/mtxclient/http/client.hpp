#pragma once

/// @file
/// @brief Primary header to access the http API.

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
#include "mtx/responses/empty.hpp" // for Empty, Logout, RoomInvite
#include "mtx/secret_storage.hpp"
#include "mtxclient/http/errors.hpp" // for ClientError
#include "mtxclient/utils.hpp"       // for random_token, url_encode, des...
// #include "mtx/common.hpp"

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
struct KeySignaturesUpload;
struct Login;
struct QueryKeys;
struct ClaimKeys;
struct UploadKeys;
struct PublicRoomVisibility;
struct PublicRooms;
}
namespace responses {
struct AvatarUrl;
struct ClaimKeys;
struct ContentURI;
struct CreateRoom;
struct EventId;
struct RoomId;
struct FilterId;
struct GroupId;
struct GroupProfile;
struct JoinedGroups;
struct KeyChanges;
struct KeySignaturesUpload;
struct Login;
struct LoginFlows;
struct Messages;
struct Notifications;
struct Profile;
struct QueryKeys;
struct Register;
struct Sync;
struct TurnServer;
struct UploadKeys;
struct Versions;
struct WellKnown;
struct PublicRoomVisibility;
struct PublicRooms;
namespace backup {
struct SessionBackup;
struct RoomKeysBackup;
struct KeysBackup;
struct BackupVersion;
}
}
}

namespace mtx {
//! Types related to invoking the actual HTTP requests
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
        //! Explicitly set the presence of the user
        std::optional<mtx::presence::PresenceState> set_presence;
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
        void join_room(const std::string &room, Callback<mtx::responses::RoomId> cb);
        //! Join a room by an alias or a room_id. `via` are other servers, that may know about this
        //! room.
        void join_room(const std::string &room,
                       const std::vector<std::string> &via,
                       Callback<mtx::responses::RoomId> cb);
        //! Leave a room by its room_id.
        void leave_room(const std::string &room_id, Callback<mtx::responses::Empty> cb);
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

        //! Get presence of a user
        void presence_status(const std::string &user_id,
                             Callback<mtx::events::presence::Presence> cb);
        //! Set presence of the user
        void put_presence_status(mtx::presence::PresenceState state,
                                 std::optional<std::string> status_msg,
                                 ErrCallback cb);

        //! Get a single event.
        void get_event(const std::string &room_id,
                       const std::string &event_id,
                       Callback<mtx::events::collections::TimelineEvents> cb);
        //! Send a room message with auto-generated transaction id.
        template<class Payload>
        void send_room_message(const std::string &room_id,
                               const Payload &payload,
                               Callback<mtx::responses::EventId> cb);
        //! Send a room message by providing transaction id.
        template<class Payload>
        void send_room_message(const std::string &room_id,
                               const std::string &txn_id,
                               const Payload &payload,
                               Callback<mtx::responses::EventId> cb);
        //! Send a state event by providing the state key.
        template<class Payload>
        void send_state_event(const std::string &room_id,
                              const std::string &state_key,
                              const Payload &payload,
                              Callback<mtx::responses::EventId> cb);
        //! Send a state event with an empty state key.
        template<class Payload>
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
        template<typename EventContent>
        void send_to_device(
          const std::string &txid,
          const std::map<mtx::identifiers::User, std::map<std::string, EventContent>> &messages,
          ErrCallback callback);

        //! Gets the visibility of a given room on the server's public room directory.
        void get_room_visibility(const std::string &room_id,
                                 Callback<mtx::responses::PublicRoomVisibility> cb);

        //! Sets the visibility of a given room in the server's public room directory.
        void put_room_visibility(const std::string &room_id,
                                 const mtx::requests::PublicRoomVisibility &req,
                                 ErrCallback cb);

        //! Lists the public rooms on the server. This API returns paginated responses.
        //! The rooms are ordered by the number of joined members, with the largest rooms first.
        void get_public_rooms(Callback<mtx::responses::PublicRooms> cb,
                              const std::string &server = "",
                              size_t limit              = 0,
                              const std::string &since  = "");

        //! Lists the public rooms on the server, with optional filter. POST Request.
        void post_public_rooms(const mtx::requests::PublicRooms &req,
                               Callback<mtx::responses::PublicRooms> cb,
                               const std::string &server = "");

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

        //! Enable encryption in a room by sending a `m.room.encryption` state event.
        void enable_encryption(const std::string &room, Callback<mtx::responses::EventId> cb);

        //! Upload identity keys & one time keys.
        void upload_keys(const mtx::requests::UploadKeys &req,
                         Callback<mtx::responses::UploadKeys> cb);

        //! Upload signatures for cross-signing keys
        void keys_signatures_upload(const mtx::requests::KeySignaturesUpload &req,
                                    Callback<mtx::responses::KeySignaturesUpload> cb);

        //! Returns the current devices and identity keys for the given users.
        void query_keys(const mtx::requests::QueryKeys &req,
                        Callback<mtx::responses::QueryKeys> cb);

        /// @brief Claims one-time keys for use in pre-key messages.
        ///
        /// Pass in a map from userid to device_keys
        void claim_keys(const mtx::requests::ClaimKeys &req,
                        Callback<mtx::responses::ClaimKeys> cb);

        /// @brief Gets a list of users who have updated their device identity keys since a previous
        /// sync token.
        void key_changes(const std::string &from,
                         const std::string &to,
                         Callback<mtx::responses::KeyChanges> cb);

        //
        // Key backup endpoints
        //
        void backup_version(Callback<mtx::responses::backup::BackupVersion> cb);
        void backup_version(const std::string &version,
                            Callback<mtx::responses::backup::BackupVersion> cb);
        void room_keys(std::string version, Callback<mtx::responses::backup::KeysBackup> cb);
        void room_keys(std::string version,
                       const std::string room_id,
                       Callback<mtx::responses::backup::RoomKeysBackup> cb);
        void room_keys(std::string version,
                       const std::string room_id,
                       const std::string session_id,
                       Callback<mtx::responses::backup::SessionBackup> cb);

        //
        // Secret storage endpoints
        //

        //! Retrieve a specific secret
        void secret_storage_secret(const std::string &secret_id,
                                   Callback<mtx::secret_storage::Secret> cb);
        //! Retrieve information about a key
        void secret_storage_key(const std::string &key_id,
                                Callback<mtx::secret_storage::AesHmacSha2KeyDescription> cb);

        //! Gets any TURN server URIs and authentication credentials
        void get_turn_server(Callback<mtx::responses::TurnServer> cb);

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

template<class Payload>
[[gnu::used, llvm::used]] void
mtx::http::Client::send_room_message(const std::string &room_id,
                                     const Payload &payload,
                                     Callback<mtx::responses::EventId> callback)
{
        send_room_message<Payload>(room_id, generate_txn_id(), payload, callback);
}

template<class Payload>
[[gnu::used, llvm::used]] void
mtx::http::Client::send_room_message(const std::string &room_id,
                                     const std::string &txn_id,
                                     const Payload &payload,
                                     Callback<mtx::responses::EventId> callback)
{
        constexpr auto event_type = mtx::events::message_content_to_type<Payload>;
        static_assert(event_type != mtx::events::EventType::Unsupported);

        const auto api_path = "/client/r0/rooms/" + mtx::client::utils::url_encode(room_id) +
                              "/send/" + mtx::events::to_string(event_type) + "/" +
                              mtx::client::utils::url_encode(txn_id);

        put<Payload, mtx::responses::EventId>(api_path, payload, callback);
}

template<class Payload>
[[gnu::used, llvm::used]] void
mtx::http::Client::send_state_event(const std::string &room_id,
                                    const std::string &state_key,
                                    const Payload &payload,
                                    Callback<mtx::responses::EventId> callback)
{
        constexpr auto event_type = mtx::events::state_content_to_type<Payload>;
        static_assert(event_type != mtx::events::EventType::Unsupported);

        const auto api_path = "/client/r0/rooms/" + mtx::client::utils::url_encode(room_id) +
                              "/state/" + mtx::events::to_string(event_type) + "/" +
                              mtx::client::utils::url_encode(state_key);

        put<Payload, mtx::responses::EventId>(api_path, payload, callback);
}

template<class Payload>
[[gnu::used, llvm::used]] void
mtx::http::Client::send_state_event(const std::string &room_id,
                                    const Payload &payload,
                                    Callback<mtx::responses::EventId> callback)
{
        send_state_event<Payload>(room_id, "", payload, callback);
}
