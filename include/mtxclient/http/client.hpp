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

#include <cstdint>    // for uint16_t, uint64_t
#include <functional> // for function
#include <memory>     // for allocator, shared_ptr, enable...
#include <optional>   // for optional
#include <string>     // for string, operator+, char_traits
#include <utility>    // for move
#include <vector>     // for vector

#include <coeurl/headers.hpp>

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
struct PushersData;
struct SetPushers;
}
namespace responses {
struct Aliases;
struct Available;
struct AvatarUrl;
struct ClaimKeys;
struct ContentURI;
struct CreateRoom;
struct Device;
struct EventId;
struct FilterId;
struct KeyChanges;
struct KeySignaturesUpload;
struct Login;
struct LoginFlows;
struct Members;
struct Messages;
struct Notifications;
struct Profile;
struct PublicRoomVisibility;
struct PublicRoomsChunk;
struct PublicRooms;
struct HierarchyRooms;
struct QueryDevices;
struct QueryKeys;
struct Register;
struct RegistrationTokenValidity;
struct RequestToken;
struct RoomId;
struct Success;
struct Sync;
struct StateEvents;
struct TurnServer;
struct UploadKeys;
struct Version;
struct Versions;
struct WellKnown;
namespace backup {
struct SessionBackup;
struct RoomKeysBackup;
struct KeysBackup;
struct BackupVersion;
}
namespace capabilities {
struct Capabilities;
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
using HeaderFields = const std::optional<coeurl::Headers> &;
using ErrCallback  = std::function<void(RequestErr)>;

template<class Response>
using Callback = std::function<void(const Response &, RequestErr)>;

template<class Response>
using HeadersCallback    = std::function<void(const Response &, HeaderFields, RequestErr)>;
using TypeErasedCallback = std::function<void(HeaderFields, const std::string_view &, int, int)>;

//! A helper to handle user interactive authentication. This will cache the request and call the
//! prompt every time there is a new stage. Advance the flow by calling next().
class UIAHandler
{
public:
    //! The callback for when a new UIA stage needs to be completed
    using UIAPrompt =
      std::function<void(const UIAHandler &, const user_interactive::Unauthorized &)>;

    //! Create a new UIA handler. Pass a callback for when a new stage needs to be completed.
    UIAHandler(UIAPrompt prompt_)
      : prompt(std::move(prompt_))
    {}

    void next(const user_interactive::Auth &auth) const;

private:
    UIAPrompt prompt;

    std::function<void(const UIAHandler &, const nlohmann::json &)> next_;

    friend class Client;
};

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
    //! Enable or disable certificate verification. On by default
    void verify_certificates(bool enabled = true);
    //! Set the homeserver domain name.
    void set_user(const mtx::identifiers::User &user) { user_id_ = user; }
    //! Set the device ID.
    void set_device_id(const std::string &device_id) { device_id_ = device_id; }
    //! Set the homeserver domain name.
    void set_server(const std::string &server);
    //! Retrieve the homeserver domain name.
    std::string server() { return server_; };
    //! Retrieve the full server url including protocol and ports
    std::string server_url()
    {
        return protocol_ + "://" + server() + ":" + std::to_string(port());
    };
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
    //! Get url to navigate to for sso login flow, optionally preselecting an identity provider
    //! Open this in a browser
    std::string login_sso_redirect(std::string redirectUrl, const std::string &idp = "");
    //! Lookup real server to connect to.
    //! Call set_server with the returned homeserver url after this
    void well_known(Callback<mtx::responses::WellKnown> cb);

    //! Check for username availability
    void register_username_available(const std::string &username,
                                     Callback<mtx::responses::Available> cb);

    //! Register with an UIA handler so you don't need to repeat the request manually.
    void registration(const std::string &user,
                      const std::string &pass,
                      UIAHandler uia_handler,
                      Callback<mtx::responses::Register> cb,
                      const std::string &initial_device_display_name = "");

    //! Send a dummy registration request to query the auth flows
    void registration(Callback<mtx::responses::Register> cb);

    //! Check the validity of a registration token
    void registration_token_validity(const std::string token,
                                     Callback<mtx::responses::RegistrationTokenValidity> cb);

    //! Validate an unused email address.
    void register_email_request_token(const requests::RequestEmailToken &r,
                                      Callback<mtx::responses::RequestToken> cb);
    //! Validate a used email address.
    void verify_email_request_token(const requests::RequestEmailToken &r,
                                    Callback<mtx::responses::RequestToken> cb);

    //! Validate an unused phone number.
    void register_phone_request_token(const requests::RequestMSISDNToken &r,
                                      Callback<mtx::responses::RequestToken> cb);
    //! Validate a used phone number.
    void verify_phone_request_token(const requests::RequestMSISDNToken &r,
                                    Callback<mtx::responses::RequestToken> cb);

    //! Validate ownership of an email address/phone number.
    void validate_submit_token(const std::string &url,
                               const requests::IdentitySubmitToken &r,
                               Callback<mtx::responses::Success>);

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
                   Callback<mtx::responses::RoomId> cb,
                   const std::string &reason = "");
    //! Leave a room by its room_id.
    void leave_room(const std::string &room_id,
                    Callback<mtx::responses::Empty> cb,
                    const std::string &reason = "");
    //! Knock on a room.
    void knock_room(const std::string &room_id,
                    const std::vector<std::string> &via,
                    Callback<mtx::responses::RoomId> cb,
                    const std::string &reason = "");

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

    //! List members in a room.
    void members(const std::string &room_id,
                 Callback<mtx::responses::Members> cb,
                 const std::string &at                                        = "",
                 std::optional<mtx::events::state::Membership> membership     = {},
                 std::optional<mtx::events::state::Membership> not_membership = {});

    //! Paginate through room messages.
    void messages(const MessagesOpts &opts, Callback<mtx::responses::Messages> cb);

    //! Get the supported versions from the server.
    void versions(Callback<mtx::responses::Versions> cb);

    //! Get the supported capabilities from the server.
    void capabilities(Callback<mtx::responses::capabilities::Capabilities> cb);

    //! Mark an event as read.
    void read_event(const std::string &room_id,
                    const std::string &event_id,
                    ErrCallback cb,
                    bool hidden = false);

    //! Redact an event from a room.
    void redact_event(const std::string &room_id,
                      const std::string &event_id,
                      Callback<mtx::responses::EventId> cb,
                      const std::string &reason = "");

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
    std::string mxc_to_download_url(const std::string &mxc_url);

    //! Retrieve a thumbnail from the given mxc url.
    //! If the thumbnail isn't found and `try_download` is `true` it will try
    //! to use the `/download` endpoint to retrieve the media.
    void get_thumbnail(const ThumbOpts &opts, Callback<std::string> cb, bool try_download = true);

    //! Send typing notifications to the room.
    void start_typing(const std::string &room_id, uint64_t timeout, ErrCallback cb);
    //! Remove typing notifications from the room.
    void stop_typing(const std::string &room_id, ErrCallback cb);

    //! Get presence of a user
    void presence_status(const std::string &user_id, Callback<mtx::events::presence::Presence> cb);
    //! Set presence of the user
    void put_presence_status(mtx::presence::PresenceState state,
                             std::optional<std::string> status_msg,
                             ErrCallback cb);

    //! Get a single event.
    void get_event(const std::string &room_id,
                   const std::string &event_id,
                   Callback<mtx::events::collections::TimelineEvents> cb);

    //! Retrieve the whole state of a room
    void get_state(const std::string &room_id, Callback<mtx::responses::StateEvents> payload);

    //! Retrieve a single state event.
    template<class Payload>
    void get_state_event(const std::string &room_id,
                         const std::string &type,
                         const std::string &state_key,
                         Callback<Payload> payload);
    //! Retrieve a single state event.
    template<class Payload>
    void get_state_event(const std::string &room_id,
                         const std::string &state_key,
                         Callback<Payload> cb);

    //! Store a room account_data event.
    template<class Payload>
    void put_room_account_data(const std::string &room_id,
                               const std::string &type,
                               const Payload &payload,
                               ErrCallback cb);
    //! Store a room account_data event.
    template<class Payload>
    void put_room_account_data(const std::string &room_id, const Payload &payload, ErrCallback cb);

    //! Store an account_data event.
    template<class Payload>
    void put_account_data(const std::string &type, const Payload &payload, ErrCallback cb);
    //! Store an account_data event.
    template<class Payload>
    void put_account_data(const Payload &payload, ErrCallback cb);

    //! Retrieve a room account_data event.
    template<class Payload>
    void get_room_account_data(const std::string &room_id,
                               const std::string &type,
                               Callback<Payload> payload);
    //! Retrieve a room account_data event.
    template<class Payload>
    void get_room_account_data(const std::string &room_id, Callback<Payload> cb);

    //! Retrieve an account_data event.
    template<class Payload>
    void get_account_data(const std::string &type, Callback<Payload> payload);
    //! Retrieve an account_data event.
    template<class Payload>
    void get_account_data(Callback<Payload> cb);

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
    void send_state_event(const std::string &room_id,
                          const std::string &event_type,
                          const std::string &state_key,
                          const nlohmann::json &payload,
                          Callback<mtx::responses::EventId> callback);
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
    void send_to_device(const std::string &event_type, const nlohmann::json &body, ErrCallback cb)
    {
        send_to_device(event_type, generate_txn_id(), body, cb);
    }
    //! Send send-to-device events to a set of client devices with a specified transaction id.
    template<typename EventContent>
    void send_to_device(
      const std::string &txid,
      const std::map<mtx::identifiers::User, std::map<std::string, EventContent>> &messages,
      ErrCallback callback);

    //! Resolve the specified roomalias to a roomid.
    void resolve_room_alias(const std::string &alias, Callback<mtx::responses::RoomId> cb);
    //! Add an alias to a room.
    void add_room_alias(const std::string &alias, const std::string &roomid, ErrCallback cb);
    //! Delete an alias from a room.
    void delete_room_alias(const std::string &alias, ErrCallback cb);
    //! List the local aliases on the users server.
    void list_room_aliases(const std::string &room_id, Callback<mtx::responses::Aliases> cb);

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

    //! Paginates over the space tree in a depth-first manner to locate child rooms of a given
    //! space.
    void get_hierarchy(const std::string &room_id,
                       Callback<mtx::responses::HierarchyRooms> cb,
                       const std::string &from = "",
                       size_t limit            = 0,
                       size_t max_depth        = 0,
                       bool suggested_only     = false);

    //! summarize a room
    void get_summary(const std::string &room_id,
                     Callback<mtx::responses::PublicRoomsChunk> cb,
                     std::vector<std::string> vias = {});

    //
    // Device related endpoints.
    //

    //! List devices
    void query_devices(Callback<mtx::responses::QueryDevices> cb);

    //! Gets information on a single device, by device id.
    void get_device(const std::string &device_id, Callback<mtx::responses::Device> cb);

    //! Updates the display name of the given device id.
    void set_device_name(const std::string &device_id,
                         const std::string &display_name,
                         ErrCallback callback);

    //! Delete device
    void delete_device(const std::string &device_id, UIAHandler uia_handler, ErrCallback cb);

    //! Delete devices
    void delete_devices(const std::vector<std::string> &device_ids,
                        UIAHandler uia_handler,
                        ErrCallback cb);

    //
    // Encryption related endpoints.
    //

    //! Enable encryption in a room by sending a `m.room.encryption` state event.
    void enable_encryption(const std::string &room, Callback<mtx::responses::EventId> cb);

    //! Upload identity keys & one time keys.
    void upload_keys(const mtx::requests::UploadKeys &req, Callback<mtx::responses::UploadKeys> cb);

    //! Upload signatures for cross-signing keys
    void keys_signatures_upload(const mtx::requests::KeySignaturesUpload &req,
                                Callback<mtx::responses::KeySignaturesUpload> cb);

    //! Upload cross signing keys
    void device_signing_upload(const mtx::requests::DeviceSigningUpload &,
                               UIAHandler uia_handler,
                               ErrCallback cb);

    //! Returns the current devices and identity keys for the given users.
    void query_keys(const mtx::requests::QueryKeys &req, Callback<mtx::responses::QueryKeys> cb);

    /// @brief Claims one-time keys for use in pre-key messages.
    ///
    /// Pass in a map from userid to device_keys
    void claim_keys(const mtx::requests::ClaimKeys &req, Callback<mtx::responses::ClaimKeys> cb);

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
    void update_backup_version(const std::string &version,
                               const mtx::responses::backup::BackupVersion &data,
                               ErrCallback cb);
    void post_backup_version(const std::string &algorithm,
                             const std::string &auth_data,
                             Callback<mtx::responses::Version> cb);

    void room_keys(const std::string &version, Callback<mtx::responses::backup::KeysBackup> cb);
    void room_keys(const std::string &version,
                   const std::string &room_id,
                   Callback<mtx::responses::backup::RoomKeysBackup> cb);
    void room_keys(const std::string &version,
                   const std::string &room_id,
                   const std::string &session_id,
                   Callback<mtx::responses::backup::SessionBackup> cb);
    void put_room_keys(const std::string &version,
                       const mtx::responses::backup::KeysBackup &keys,
                       ErrCallback cb);
    void put_room_keys(const std::string &version,
                       const std::string &room_id,
                       const mtx::responses::backup::RoomKeysBackup &keys,
                       ErrCallback cb);
    void put_room_keys(const std::string &version,
                       const std::string &room_id,
                       const std::string &session_id,
                       const mtx::responses::backup::SessionBackup &keys,
                       ErrCallback cb);

    //
    // Secret storage endpoints
    //

    //! Retrieve a specific secret
    void secret_storage_secret(const std::string &secret_id,
                               Callback<mtx::secret_storage::Secret> cb);
    //! Retrieve information about a key
    void secret_storage_key(const std::string &key_id,
                            Callback<mtx::secret_storage::AesHmacSha2KeyDescription> cb);

    //! Upload a specific secret
    void upload_secret_storage_secret(const std::string &secret_id,
                                      const mtx::secret_storage::Secret &secret,
                                      ErrCallback cb);
    //! Upload information about a key
    void upload_secret_storage_key(const std::string &key_id,
                                   const mtx::secret_storage::AesHmacSha2KeyDescription &desc,
                                   ErrCallback cb);

    //! Set the default key for the secret storage
    void set_secret_storage_default_key(const std::string &key_id, ErrCallback cb);

    //! Gets any TURN server URIs and authentication credentials
    void get_turn_server(Callback<mtx::responses::TurnServer> cb);

    //! Sets, updates, or deletes a pusher
    void set_pusher(const mtx::requests::SetPusher &req, Callback<mtx::responses::Empty> cb);

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
             const std::string &endpoint_namespace = "/_matrix",
             int num_redirects                     = 0);

    // type erased versions of http verbs
    void post(const std::string &endpoint,
              const std::string &req,
              TypeErasedCallback cb,
              bool requires_auth,
              const std::string &content_type);

    void put(const std::string &endpoint,
             const std::string &req,
             TypeErasedCallback cb,
             bool requires_auth);

    void get(const std::string &endpoint,
             TypeErasedCallback cb,
             bool requires_auth,
             const std::string &endpoint_namespace,
             int num_redirects = 0);

    void delete_(const std::string &endpoint, ErrCallback cb, bool requires_auth = true);

    coeurl::Headers prepare_headers(bool requires_auth);
    std::string endpoint_to_url(const std::string &endpoint,
                                const char *endpoint_namespace = "/_matrix");

    template<class Response>
    TypeErasedCallback prepare_callback(HeadersCallback<Response> callback);

    //! The protocol used, i.e. https or http
    std::string protocol_;
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

// Template instantiations for the various send functions

#define MTXCLIENT_SEND_STATE_EVENT_FWD(Content)                                                    \
    extern template void mtx::http::Client::send_state_event<mtx::events::Content>(                \
      const std::string &,                                                                         \
      const std::string &state_key,                                                                \
      const mtx::events::Content &,                                                                \
      Callback<mtx::responses::EventId> cb);                                                       \
    extern template void mtx::http::Client::send_state_event<mtx::events::Content>(                \
      const std::string &, const mtx::events::Content &, Callback<mtx::responses::EventId> cb);

MTXCLIENT_SEND_STATE_EVENT_FWD(state::Aliases)
MTXCLIENT_SEND_STATE_EVENT_FWD(state::Avatar)
MTXCLIENT_SEND_STATE_EVENT_FWD(state::CanonicalAlias)
MTXCLIENT_SEND_STATE_EVENT_FWD(state::Create)
MTXCLIENT_SEND_STATE_EVENT_FWD(state::Encryption)
MTXCLIENT_SEND_STATE_EVENT_FWD(state::GuestAccess)
MTXCLIENT_SEND_STATE_EVENT_FWD(state::HistoryVisibility)
MTXCLIENT_SEND_STATE_EVENT_FWD(state::JoinRules)
MTXCLIENT_SEND_STATE_EVENT_FWD(state::Member)
MTXCLIENT_SEND_STATE_EVENT_FWD(state::Name)
MTXCLIENT_SEND_STATE_EVENT_FWD(state::PinnedEvents)
MTXCLIENT_SEND_STATE_EVENT_FWD(state::PowerLevels)
MTXCLIENT_SEND_STATE_EVENT_FWD(state::Tombstone)
MTXCLIENT_SEND_STATE_EVENT_FWD(state::Topic)
MTXCLIENT_SEND_STATE_EVENT_FWD(state::Widget)
MTXCLIENT_SEND_STATE_EVENT_FWD(state::policy_rule::UserRule)
MTXCLIENT_SEND_STATE_EVENT_FWD(state::policy_rule::RoomRule)
MTXCLIENT_SEND_STATE_EVENT_FWD(state::policy_rule::ServerRule)
MTXCLIENT_SEND_STATE_EVENT_FWD(state::space::Child)
MTXCLIENT_SEND_STATE_EVENT_FWD(state::space::Parent)
MTXCLIENT_SEND_STATE_EVENT_FWD(msc2545::ImagePack)

#define MTXCLIENT_SEND_ROOM_MESSAGE_FWD(Content)                                                   \
    extern template void mtx::http::Client::send_room_message<Content>(                            \
      const std::string &,                                                                         \
      const std::string &,                                                                         \
      const Content &,                                                                             \
      Callback<mtx::responses::EventId> cb);                                                       \
    extern template void mtx::http::Client::send_room_message<Content>(                            \
      const std::string &, const Content &, Callback<mtx::responses::EventId> cb);

MTXCLIENT_SEND_ROOM_MESSAGE_FWD(mtx::events::msg::Encrypted)
MTXCLIENT_SEND_ROOM_MESSAGE_FWD(mtx::events::msg::StickerImage)
MTXCLIENT_SEND_ROOM_MESSAGE_FWD(mtx::events::msg::Reaction)
MTXCLIENT_SEND_ROOM_MESSAGE_FWD(mtx::events::msg::Audio)
MTXCLIENT_SEND_ROOM_MESSAGE_FWD(mtx::events::msg::Emote)
MTXCLIENT_SEND_ROOM_MESSAGE_FWD(mtx::events::msg::File)
MTXCLIENT_SEND_ROOM_MESSAGE_FWD(mtx::events::msg::Image)
MTXCLIENT_SEND_ROOM_MESSAGE_FWD(mtx::events::msg::Notice)
MTXCLIENT_SEND_ROOM_MESSAGE_FWD(mtx::events::msg::Text)
MTXCLIENT_SEND_ROOM_MESSAGE_FWD(mtx::events::msg::Video)
// MTXCLIENT_SEND_ROOM_MESSAGE(mtx::events::msg::KeyVerificationRequest)
// MTXCLIENT_SEND_ROOM_MESSAGE(mtx::events::msg::KeyVerificationStart)
// MTXCLIENT_SEND_ROOM_MESSAGE(mtx::events::msg::KeyVerificationReady)
// MTXCLIENT_SEND_ROOM_MESSAGE(mtx::events::msg::KeyVerificationDone)
// MTXCLIENT_SEND_ROOM_MESSAGE(mtx::events::msg::KeyVerificationAccept)
// MTXCLIENT_SEND_ROOM_MESSAGE(mtx::events::msg::KeyVerificationCancel)
// MTXCLIENT_SEND_ROOM_MESSAGE(mtx::events::msg::KeyVerificationKey)
// MTXCLIENT_SEND_ROOM_MESSAGE(mtx::events::msg::KeyVerificationMac)
MTXCLIENT_SEND_ROOM_MESSAGE_FWD(mtx::events::voip::CallInvite)
MTXCLIENT_SEND_ROOM_MESSAGE_FWD(mtx::events::voip::CallCandidates)
MTXCLIENT_SEND_ROOM_MESSAGE_FWD(mtx::events::voip::CallAnswer)
MTXCLIENT_SEND_ROOM_MESSAGE_FWD(mtx::events::voip::CallHangUp)
MTXCLIENT_SEND_ROOM_MESSAGE_FWD(mtx::events::voip::CallSelectAnswer)
MTXCLIENT_SEND_ROOM_MESSAGE_FWD(mtx::events::voip::CallReject)
MTXCLIENT_SEND_ROOM_MESSAGE_FWD(mtx::events::voip::CallNegotiate)

#define MTXCLIENT_SEND_TO_DEVICE_FWD(Content)                                                      \
    extern template void mtx::http::Client::send_to_device<Content>(                               \
      const std::string &txid,                                                                     \
      const std::map<mtx::identifiers::User, std::map<std::string, Content>> &messages,            \
      ErrCallback callback);

MTXCLIENT_SEND_TO_DEVICE_FWD(mtx::events::msg::RoomKey)
MTXCLIENT_SEND_TO_DEVICE_FWD(mtx::events::msg::ForwardedRoomKey)
MTXCLIENT_SEND_TO_DEVICE_FWD(mtx::events::msg::KeyRequest)
MTXCLIENT_SEND_TO_DEVICE_FWD(mtx::events::msg::OlmEncrypted)
MTXCLIENT_SEND_TO_DEVICE_FWD(mtx::events::msg::Encrypted)
MTXCLIENT_SEND_TO_DEVICE_FWD(mtx::events::msg::Dummy)
MTXCLIENT_SEND_TO_DEVICE_FWD(mtx::events::msg::KeyVerificationRequest)
MTXCLIENT_SEND_TO_DEVICE_FWD(mtx::events::msg::KeyVerificationStart)
MTXCLIENT_SEND_TO_DEVICE_FWD(mtx::events::msg::KeyVerificationReady)
MTXCLIENT_SEND_TO_DEVICE_FWD(mtx::events::msg::KeyVerificationDone)
MTXCLIENT_SEND_TO_DEVICE_FWD(mtx::events::msg::KeyVerificationAccept)
MTXCLIENT_SEND_TO_DEVICE_FWD(mtx::events::msg::KeyVerificationCancel)
MTXCLIENT_SEND_TO_DEVICE_FWD(mtx::events::msg::KeyVerificationKey)
MTXCLIENT_SEND_TO_DEVICE_FWD(mtx::events::msg::KeyVerificationMac)
MTXCLIENT_SEND_TO_DEVICE_FWD(mtx::events::msg::SecretSend)
MTXCLIENT_SEND_TO_DEVICE_FWD(mtx::events::msg::SecretRequest)

#define MTXCLIENT_ACCOUNT_DATA_FWD(Payload)                                                        \
    extern template void mtx::http::Client::put_room_account_data<Payload>(                        \
      const std::string &room_id,                                                                  \
      const std::string &type,                                                                     \
      const Payload &payload,                                                                      \
      ErrCallback cb);                                                                             \
    extern template void mtx::http::Client::put_room_account_data<Payload>(                        \
      const std::string &room_id, const Payload &payload, ErrCallback cb);                         \
    extern template void mtx::http::Client::put_account_data<Payload>(                             \
      const std::string &type, const Payload &payload, ErrCallback cb);                            \
    extern template void mtx::http::Client::put_account_data<Payload>(const Payload &payload,      \
                                                                      ErrCallback cb);             \
    extern template void mtx::http::Client::get_room_account_data<Payload>(                        \
      const std::string &room_id, const std::string &type, Callback<Payload> payload);             \
    extern template void mtx::http::Client::get_room_account_data<Payload>(                        \
      const std::string &room_id, Callback<Payload> cb);                                           \
    extern template void mtx::http::Client::get_account_data<Payload>(const std::string &type,     \
                                                                      Callback<Payload> payload);  \
    extern template void mtx::http::Client::get_account_data<Payload>(Callback<Payload> cb);

MTXCLIENT_ACCOUNT_DATA_FWD(mtx::events::msc2545::ImagePack)
MTXCLIENT_ACCOUNT_DATA_FWD(mtx::events::msc2545::ImagePackRooms)
MTXCLIENT_ACCOUNT_DATA_FWD(mtx::events::account_data::nheko_extensions::HiddenEvents)
MTXCLIENT_ACCOUNT_DATA_FWD(mtx::events::account_data::Tags)
MTXCLIENT_ACCOUNT_DATA_FWD(mtx::events::account_data::Direct)
