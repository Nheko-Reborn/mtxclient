#pragma once

/// @file
/// @brief Structs for for requests to the Matrix API.

#include <string>
#include <variant>

#include <mtx/common.hpp>
#include <mtx/events/collections.hpp>
#if __has_include(<nlohmann/json_fwd.hpp>)
#include <nlohmann/json_fwd.hpp>
#else
#include <nlohmann/json.hpp>
#endif

namespace mtx {
//! Namespace for request structs
namespace requests {
//! Convenience parameter for setting various default state events based on a preset.
enum class Preset
{
    //! `join_rules` is set to `invite`. `history_visibility` is set to `shared`.
    PrivateChat,
    //! `join_rules` is set to `public`. `history_visibility` is set to `shared`.
    PublicChat,
    //! `join_rules` is set to `invite`. `history_visibility` is set to `shared`.
    //! All invitees are given the same power level as the room creator.
    TrustedPrivateChat,
};

//! Request payload for the `POST /_matrix/client/r0/createRoom` endpoint.
struct CreateRoom
{
    //! If this is included, an `m.room.name` event will
    //! be sent into the room to indicate the name of the room.
    std::string name;
    //! If this is included, an `m.room.topic` event will be sent
    //! into the room to indicate the topic for the room.
    std::string topic;
    //! The desired room alias local part. e.g `#foo:example.com`.
    std::string room_alias_name;
    //! A list of user IDs to invite to the room.
    std::vector<std::string> invite;
    //! This flag makes the server set the is_direct flag on the
    //! `m.room.member` events sent to the users in @p invite and @p invite_3pid.
    bool is_direct = false;
    //! Convenience parameter for setting various default state events.
    Preset preset = Preset::PrivateChat;
    //! Whether or not the room will be visible by non members.
    common::RoomVisibility visibility = common::RoomVisibility::Private;

    //! A list of state events to set in the new room. This allows the user to override the default
    //! state events set in the new room. The expected format of the state events are an object with
    //! type, state_key and content keys set.
    std::vector<events::collections::StrippedEvents> initial_state;

    //! The room version to set for the room. If not provided, the homeserver is to use its
    //! configured default. If provided, the homeserver will return a 400 error with the errcode
    //! M_UNSUPPORTED_ROOM_VERSION if it does not support the room version.
    std::string room_version;

    //! Extra keys, such as m.federate, to be added to the content of the m.room.create event. The
    //! server will overwrite the following keys: creator, room_version. Future versions of the
    //! specification may allow the server to overwrite other keys.
    std::optional<events::state::Create> creation_content;

    friend void to_json(nlohmann::json &obj, const CreateRoom &request);
};

namespace login_identifier {
//! The user is identified by their Matrix ID.
struct User
{
    //! A client can identify a user using their Matrix ID. This can either be the fully
    //! qualified Matrix user ID, or just the localpart of the user ID.
    std::string user;
};
//! The user is identified by a third-party identifier in canonicalised form.
struct Thirdparty
{
    //! The medium of the third party identifier
    std::string medium;
    //! The canonicalised third party address of the user
    std::string address;
};
//! The user is identified by a phone number.
struct PhoneNumber
{
    //! The country is the two-letter uppercase ISO-3166-1 alpha-2 country code that the number
    //! in phone should be parsed as if it were dialled from.
    std::string country;
    //! The phone number. If the client wishes to canonicalise the phone number, then it can use
    //! the m.id.thirdparty identifier type with a medium of msisdn instead.
    std::string phone;
};
}

//! Request payload for the `POST /_matrix/client/r0/login` endpoint.
struct Login
{
    //! The login type being used. One of ["m.login.password", "m.login.token"]
    std::string type = "m.login.password";
    //! Identification information for the user. Usually you use `login_identifier::User` with
    //! an mxid. Required.
    std::
      variant<login_identifier::User, login_identifier::Thirdparty, login_identifier::PhoneNumber>
        identifier;
    //! Required when type is m.login.token. The login token.
    std::string token;
    //! Required when type is m.login.password. The user's password.
    std::string password;
    //! ID of the client device. If this does not correspond to a known client device,
    //! a new device will be created.
    //! The server will auto-generate a device_id if this is not specified.
    std::string device_id;
    //! A display name to assign to the newly-created device.
    //! Ignored if device_id corresponds to a known device.
    std::string initial_device_display_name;

    friend void to_json(nlohmann::json &obj, const Login &request);
};

/// @brief Request payload for
/// `POST /_matrix/client/r0/{register,account/password}/email/requestToken`
///
/// The homeserver should validate the email itself, either by sending a validation email itself or
/// by using a service it has control over.
struct RequestEmailToken
{
    //! Required. A unique string generated by the client, and used to identify the validation
    //! attempt. It must be a string consisting of the characters [0-9a-zA-Z.=_-]. Its length must
    //! not exceed 255 characters and it must not be empty.
    std::string client_secret;
    //! Required. The email address to validate.
    std::string email;

    //! Required. The server will only send an email if the send_attempt is a number greater than
    //! the most recent one which it has seen, scoped to that email + client_secret pair. This is to
    //! avoid repeatedly sending the same email in the case of request retries between the POSTing
    //! user and the identity server. The client should increment this value if they desire a new
    //! email (e.g. a reminder) to be sent. If they do not, the server should respond with success
    //! but not resend the email.
    int send_attempt = 0;

    friend void to_json(nlohmann::json &obj, const RequestEmailToken &request);
};

/// @brief Request payload for
/// `POST /_matrix/client/r0/{register,account/password}/msisdn/requestToken`
///
/// The homeserver should validate the email itself, either by sending a validation email itself or
/// by using a service it has control over.
struct RequestMSISDNToken
{
    //! Required. A unique string generated by the client, and used to identify the validation
    //! attempt. It must be a string consisting of the characters [0-9a-zA-Z.=_-]. Its length must
    //! not exceed 255 characters and it must not be empty.
    std::string client_secret;
    //! Required. The two-letter uppercase ISO-3166-1 alpha-2 country code that the number in
    //! phone_number should be parsed as if it were dialled from.
    std::string country;
    //! Required. The phone number to validate.
    std::string phone_number;

    //! Required. The server will only send an SMS if the send_attempt is a number greater than the
    //! most recent one which it has seen, scoped to that country + phone_number + client_secret
    //! triple. This is to avoid repeatedly sending the same SMS in the case of request retries
    //! between the POSTing user and the identity server. The client should increment this value if
    //! they desire a new SMS (e.g. a reminder) to be sent.
    int send_attempt = 0;
};

void
to_json(nlohmann::json &obj, const RequestMSISDNToken &request);

//! Validate ownership of an email address/phone number.
struct IdentitySubmitToken
{
    //! Required. The session ID, generated by the requestToken call.
    std::string sid;
    //! Required. The client secret that was supplied to the requestToken call.
    std::string client_secret;
    //! Required. The token generated by the requestToken call and emailed to the user.
    std::string token;

    friend void to_json(nlohmann::json &obj, const IdentitySubmitToken &request);
};

//! Request payload for the `PUT /_matrix/client/r0/sendToDevice/{eventType}/{transcationID}`
template<typename EventContent>
using ToDeviceMessages = std::map<mtx::identifiers::User, std::map<std::string, EventContent>>;

//! Request payload for the `POST /_matrix/client/r0/profile/{userId}/avatar_url` endpoint.
struct AvatarUrl
{
    std::string avatar_url;

    friend void to_json(nlohmann::json &obj, const AvatarUrl &request);
};

//! Request payload for the `PUT /_matrix/client/r0/profile/{userId}/displayname` endpoint.
struct DisplayName
{
    //! The new display name for this device. If not given, the display name is unchanged.
    std::string displayname;

    friend void to_json(nlohmann::json &obj, const DisplayName &request);
};

//! Request payload for the `POST /_matrix/client/r0/rooms/{roomId}/invite` endpoint as well as ban,
//! unban and kick.
struct RoomMembershipChange
{
    std::string user_id;

    //! optional kick, invite or ban reason
    std::string reason;

    friend void to_json(nlohmann::json &obj, const RoomMembershipChange &request);
};

//! Request payload for the `PUT /_matrix/client/r0/rooms/{roomId}/typing/{userId}` endpoint.
struct TypingNotification
{
    //! Whether the user is typing or not.
    bool typing = false;
    //! The length of time in milliseconds to mark this user as typing.
    uint64_t timeout = 0;

    friend void to_json(nlohmann::json &obj, const TypingNotification &request);
};

//! Request payload for the `PUT /_matrix/client/r0/devices/{deviceId}` endpoint.
struct DeviceUpdate
{
    std::string display_name;

    friend void to_json(nlohmann::json &obj, const DeviceUpdate &request);
};

//! Request payload for the `PUT /_matrix/client/r0/directory/list/room/{roomId}` endpoint
struct PublicRoomVisibility
{
    //! The new visibility setting for the room. Defaults to 'public'. One of: ["private",
    //! "public"]
    common::RoomVisibility visibility;

    friend void to_json(nlohmann::json &obj, const PublicRoomVisibility &request);
};

struct PublicRoomsFilter
{
    //! A string to search for in the room metadata,
    //! e.g. name, topic, canonical alias etc. (Optional).
    std::string generic_search_term;

    friend void to_json(nlohmann::json &obj, const PublicRoomsFilter &req);
};

//! Request payload for the `POST /_matrix/client/r0/publicRooms` endpoint.
struct PublicRooms
{
    //! Limit the number of results returned.
    int limit;
    //! A pagination token from a previous request, allowing clients
    //! to get the next (or previous) batch of rooms. The direction of
    //! pagination is specified solely by which token is supplied,
    //! rather than via an explicit flag.
    std::string since;
    //! Filter to apply to the results.
    PublicRoomsFilter filter;
    //! Whether or not to include all known networks/protocols from
    //! application services on the homeserver. Defaults to false.
    bool include_all_networks = false;
    //! The specific third party network/protocol to request from
    //! the homeserver. Can only be used if include_all_networks is false.
    std::string third_party_instance_id;

    friend void to_json(nlohmann::json &obj, const PublicRooms &request);
};

struct Empty
{
    friend inline void to_json(nlohmann::json &, const Empty &) {}
};

using Logout = Empty;

struct SignedOneTimeKey
{
    //! Required. The unpadded Base64-encoded 32-byte Curve25519 public key.
    std::string key;
    //! When uploading a signed key, an additional fallback: true key should be included to denote
    //! that the key is a fallback key.
    bool fallback = false;
    //! Required. Signatures of the key object.
    //! The signature is calculated using the process described at Signing JSON.
    std::map<std::string, std::map<std::string, std::string>> signatures;

    friend void to_json(nlohmann::json &obj, const SignedOneTimeKey &);
};

struct UploadKeys
{
    //! Identity keys for the device.
    //! May be absent if no new identity keys are required.
    mtx::crypto::DeviceKeys device_keys;
    //! One-time public keys for "pre-key" messages.
    //! The names of the properties should be in the format `<algorithm>:<key_id>`.
    //! The format of the key is determined by the key algorithm.
    std::map<std::string, std::variant<std::string, SignedOneTimeKey>> one_time_keys;

    //! The public key which should be used if the device’s one-time keys are exhausted. The
    //! fallback key is not deleted once used, but should be replaced when additional one-time keys
    //! are being uploaded. The server will notify the client of the fallback key being used through
    //! /sync.
    //!
    //! There can only be at most one key per algorithm uploaded, and the server will only persist
    //! one key per algorithm.
    //!
    //! When uploading a signed key, an additional fallback: true key should be included to denote
    //! that the key is a fallback key.
    //!
    //! May be absent if a new fallback key is not required.
    std::map<std::string, std::variant<std::string, SignedOneTimeKey>> fallback_keys;

    friend void to_json(nlohmann::json &obj, const UploadKeys &);
};

constexpr uint64_t DEFAULT_DOWNLOAD_TIMEOUT = 10UL * 1000; // 10 seconds

struct QueryKeys
{
    //! The time (in milliseconds) to wait when downloading keys from remote servers.
    uint64_t timeout = DEFAULT_DOWNLOAD_TIMEOUT;
    //! The keys to be downloaded.
    //! A map from user ID, to a list of device IDs, or to an empty
    //! list to indicate all devices for the corresponding user.
    std::map<std::string, std::vector<std::string>> device_keys;
    //! If the client is fetching keys as a result of a device update
    //! received in a sync request, this should be the 'since' token of
    //! that sync request, or any later sync token.
    //! This allows the server to ensure its response contains the keys
    //! advertised by the notification in that sync.
    std::string token;

    friend void to_json(nlohmann::json &obj, const QueryKeys &);
};

struct ClaimKeys
{
    //! The time (in milliseconds) to wait when downloading keys from remote servers.
    uint64_t timeout = DEFAULT_DOWNLOAD_TIMEOUT;
    //! The keys to be claimed. A map from user ID, to a map from device ID to algorithm name.
    std::map<std::string, std::map<std::string, std::string>> one_time_keys;

    friend void to_json(nlohmann::json &obj, const ClaimKeys &request);
};

struct KeySignaturesUpload
{
    //! map from user_id to either a map of device id to DeviceKey with new signatures or the
    //! key id to CrossSigningKeys with new signatures
    std::map<
      std::string,
      std::map<std::string, std::variant<mtx::crypto::DeviceKeys, mtx::crypto::CrossSigningKeys>>>
      signatures;

    friend void to_json(nlohmann::json &obj, const KeySignaturesUpload &req);
};

//! Upload cross signing keys
struct DeviceSigningUpload
{
    //! Optional. The user's master key.
    std::optional<mtx::crypto::CrossSigningKeys> master_key;
    //! Optional. The user's self-signing key. Must be signed by the accompanying master key, or by
    //! the user's most recently uploaded master key if no master key is included in the request.
    std::optional<mtx::crypto::CrossSigningKeys> self_signing_key;
    //! Optional. The user's user-signing key. Must be signed by the accompanying master key, or by
    //! the user's most recently uploaded master key if no master key is included in the request.
    std::optional<mtx::crypto::CrossSigningKeys> user_signing_key;

    friend void to_json(nlohmann::json &obj, const DeviceSigningUpload &req);
};

struct PusherData
{
    //! Required if `kind` is http. The URL to use to send notifications to.
    //! MUST be an HTTPS URL with a path of /_matrix/push/v1/notify.
    std::string url;
    //! The format to send notifications in to Push Gateways if the kind is http.
    //! The details about what fields the homeserver should send to the push gateway are
    //! defined in the Push Gateway Specification. Currently the only format available is
    //! 'event_id_only'.
    std::string format;

    friend void to_json(nlohmann::json &obj, const PusherData &data);
};

//! Request payload for the `POST /_matrix/client/r0/pushers/set` endpoint.
struct SetPusher
{
    //! Required. Unique identifier for this pusher.
    std::string pushkey;
    //! Required. The kind of pusher to configure. "http" makes a pusher that sends HTTP pokes.
    //! "email" makes a pusher that emails the user with unread notifications.
    //! null deletes the pusher.
    std::string kind;
    //! Required. This is a reverse-DNS style identifier for the application.
    //! If the `kind` is "email", this is "m.email".
    std::string app_id;
    //! Required. A string that will allow the user to identify what application owns this
    //! pusher.
    std::string app_display_name;
    //! Required. A string that will allow the user to identify what device owns this pusher.
    std::string device_display_name;
    //! Determines which set of device specific rules this pusher executes.
    std::string profile_tag;
    //! Required. The preferred language for receiving notifications.
    std::string lang;
    //! Required. Data for the pusher implementation (for example, if `kind` is `http`, includes
    //! the URL to push to).
    PusherData data;
    //! If true, add another pusher instead of updating an existing one.
    bool append = false;

    friend void to_json(nlohmann::json &obj, const SetPusher &req);
};
} // namespace requests
} // namespace mtx
