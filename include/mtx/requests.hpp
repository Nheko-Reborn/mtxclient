#pragma once

#include <string>
#include <variant>

#include <mtx/common.hpp>
#include <mtx/events/collections.hpp>
#if __has_include(<nlohmann/json_fwd.hpp>)
#include <nlohmann/json_fwd.hpp>
#else
#include <nlohmann/json.hpp>
#endif

using json = nlohmann::json;

namespace mtx {
namespace requests {

//! Whether or not the room will be visible by non members.
enum class Visibility
{
        //! A private visibility will hide the room from the published room list.
        Private,
        //! Indicates that the room will be shown in the published room list
        Public,
};

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
        Visibility visibility = Visibility::Private;
};

void
to_json(json &obj, const CreateRoom &request);

//! Request payload for the `POST /_matrix/client/r0/login` endpoint.
struct Login
{
        //! The login type being used. One of ["m.login.password", "m.login.token"]
        std::string type = "m.login.password";
        //! The fully qualified user ID or just local part of the user ID, to log in.
        std::string user;
        //! When logging in using a third party identifier, the medium of the identifier.
        //! Must be 'email'.
        std::string medium;
        //! Third party identifier for the user.
        std::string address;
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
};

void
to_json(json &obj, const Login &request);

//! Request payload for the `PUT /_matrix/client/r0/sendToDevice/{eventType}/{transcationID}`
template<typename EventContent>
using ToDeviceMessages = std::map<mtx::identifiers::User, std::map<std::string, EventContent>>;

//! Request payload for the `POST /_matrix/client/r0/profile/{userId}/avatar_url` endpoint.
struct AvatarUrl
{
        std::string avatar_url;
};

void
to_json(json &obj, const AvatarUrl &request);

//! Request payload for the `PUT /_matrix/client/r0/profile/{userId}/displayname` endpoint.
struct DisplayName
{
        std::string displayname;
};

void
to_json(json &obj, const DisplayName &request);

//! Request payload for the `POST /_matrix/client/r0/rooms/{roomId}/invite` endpoint as well as ban,
//! unban and kick.
struct RoomMembershipChange
{
        std::string user_id;

        //! optional kick, invite or ban reason
        std::string reason;
};

void
to_json(json &obj, const RoomMembershipChange &request);

//! Request payload for the `PUT /_matrix/client/r0/rooms/{roomId}/typing/{userId}` endpoint.
struct TypingNotification
{
        //! Whether the user is typing or not.
        bool typing = false;
        //! The length of time in milliseconds to mark this user as typing.
        uint64_t timeout = 0;
};

void
to_json(json &obj, const TypingNotification &request);

struct PublicRoomsFilter {
    //! A string to search for in the room metadata,
    //! e.g. name, topic, canonical alias etc. (Optional).
    std::string generic_search_term;
};

void
to_json(nlohmann::json &obj, const PublicRoomsFilter &req);

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
};

void
to_json(json &obj, const PublicRooms &request);

struct Empty
{};

inline void
to_json(json &, const Empty &)
{}

using Logout = Empty;

struct SignedOneTimeKey
{
        //! Required. The unpadded Base64-encoded 32-byte Curve25519 public key.
        std::string key;
        //! Required. Signatures of the key object.
        //! The signature is calculated using the process described at Signing JSON.
        std::map<std::string, std::map<std::string, std::string>> signatures;
};
void
to_json(json &obj, const SignedOneTimeKey &);

struct UploadKeys
{
        //! Identity keys for the device.
        //! May be absent if no new identity keys are required.
        mtx::crypto::DeviceKeys device_keys;
        //! One-time public keys for "pre-key" messages.
        //! The names of the properties should be in the format <algorithm>:<key_id>.
        //! The format of the key is determined by the key algorithm.
        std::map<std::string, std::variant<std::string, SignedOneTimeKey>> one_time_keys;
};

void
to_json(json &obj, const UploadKeys &);

constexpr uint64_t DEFAULT_DOWNLOAD_TIMEOUT = 10 * 1000; // 10 seconds

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
};

void
to_json(json &obj, const QueryKeys &);

struct ClaimKeys
{
        //! The time (in milliseconds) to wait when downloading keys from remote servers.
        uint64_t timeout = DEFAULT_DOWNLOAD_TIMEOUT;
        //! The keys to be claimed. A map from user ID, to a map from device ID to algorithm name.
        std::map<std::string, std::map<std::string, std::string>> one_time_keys;
};

void
to_json(json &obj, const ClaimKeys &request);

struct KeySignaturesUpload
{
        //! map from user_id to either a map of device id to DeviceKey with new signatures or the
        //! key id to CrossSigningKeys with new signatures
        std::map<std::string,
                 std::map<std::string,
                          std::variant<mtx::crypto::DeviceKeys, mtx::crypto::CrossSigningKeys>>>
          signatures;
};

void
to_json(json &obj, const KeySignaturesUpload &req);
} // namespace requests
} // namespace mtx
