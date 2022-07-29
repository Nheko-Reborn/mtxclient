#pragma once

/// @file
/// @brief Various event types used in E2EE.

#if __has_include(<nlohmann/json_fwd.hpp>)
#include <nlohmann/json_fwd.hpp>
#else
#include <nlohmann/json.hpp>
#endif

#include "mtx/events.hpp"
#include "mtx/events/common.hpp"
#include "variant"

namespace mtx {
namespace events {
namespace msg {

//! Display methods for Short Authentication Strings
enum class SASMethods
{
    Decimal, //!< Display 3 times 4 digits
    Emoji,   //! Display 7 emoji
    Unsupported
};

void
from_json(const nlohmann::json &obj, SASMethods &method);

void
to_json(nlohmann::json &obj, const SASMethods &method);

//! The different verification methods
enum class VerificationMethods
{
    SASv1,      //!< Short Authentication Strings
    Unsupported //!< Unsupported method
};

void
from_json(const nlohmann::json &obj, VerificationMethods &method);

void
to_json(nlohmann::json &obj, const VerificationMethods &method);

//! Content of an individual message encrypted for a certain key.
struct OlmCipherContent
{
    //! Ciphertext of the message.
    std::string body;
    /// @brief Olm message type.
    ///
    /// `0` for initial pre-key messages.
    /// `1` for normal messages after the initial exchange.
    uint8_t type;

    friend void from_json(const nlohmann::json &obj, OlmCipherContent &event);

    friend void to_json(nlohmann::json &obj, const OlmCipherContent &event);
};

//! Content of the `m.room.encrypted` Olm event.
struct OlmEncrypted
{
    //! Algorithm used for encrypting this event.
    std::string algorithm;
    //! curve25519 key of the sender.
    std::string sender_key;

    using RecipientKey = std::string;
    //! Map of recipient curve25519 key to the encrypted message.
    std::map<RecipientKey, OlmCipherContent> ciphertext;

    friend void from_json(const nlohmann::json &obj, OlmEncrypted &event);

    friend void to_json(nlohmann::json &obj, const OlmEncrypted &event);
};

//! Content of the `m.room.encrypted` event.
struct Encrypted
{
    //! Used for one-on-one exchanges.
    std::string algorithm;
    //! The actual encrypted payload.
    std::string ciphertext;
    //! Sender's device id.
    std::string device_id;
    //! The curve25519 device key.
    std::string sender_key;
    //! Outbound group session id.
    std::string session_id;
    //! Relations like rich replies
    common::Relations relations;

    friend void from_json(const nlohmann::json &obj, Encrypted &event);

    friend void to_json(nlohmann::json &obj, const Encrypted &event);
};

//! Content of the `m.dummy` event.
struct Dummy
{
    friend void from_json(const nlohmann::json &obj, Dummy &event);
    friend void to_json(nlohmann::json &obj, const Dummy &event);
};

//! Content of the `m.room_key` event.
struct RoomKey
{
    /// @brief *Required.* The encryption algorithm the key in this event is to be used with.
    ///
    /// Must be 'm.megolm.v1.aes-sha2'.
    std::string algorithm;
    std::string room_id;     //!< *Required.* The room where the key is used.
    std::string session_id;  //!< *Required.* The ID of the session that the key is for.
    std::string session_key; //!< *Required.* The key to be exchanged.

    friend void from_json(const nlohmann::json &obj, RoomKey &event);
    friend void to_json(nlohmann::json &obj, const RoomKey &event);
};

//! Content of the `m.forwarded_room_key` event.
struct ForwardedRoomKey
{
    /// @brief *Required.* The encryption algorithm the key in this event is to be used with.
    std::string algorithm;
    std::string room_id;     //!< *Required.* The room where the key is used.
    std::string session_id;  //!< *Required.* The ID of the session that the key is for.
    std::string session_key; //!< *Required.* The key to be exchanged.

    //! *Required.* The Curve25519 key of the device which initiated the session originally.
    std::string sender_key;
    /// @brief *Required.* The Ed25519 key of the device which initiated the session originally.
    ///
    /// It is 'claimed' because the receiving device has no way to tell that the original
    /// room_key actually came from a device which owns the private part of this key unless they
    /// have done device verification.
    std::string sender_claimed_ed25519_key;
    /// @brief *Required.* Chain of Curve25519 keys.
    ///
    /// It starts out empty, but each time the key is forwarded to another device, the previous
    /// sender in the chain is added to the end of the list. For example, if the key is
    /// forwarded from A to B to C, this field is empty between A and B, and contains A's
    /// Curve25519 key between B and C.
    std::vector<std::string> forwarding_curve25519_key_chain;

    friend void from_json(const nlohmann::json &obj, ForwardedRoomKey &event);
    friend void to_json(nlohmann::json &obj, const ForwardedRoomKey &event);
};

//! The type of key request.
enum class RequestAction
{
    Request,      //!< request
    Cancellation, //!< request_cancellation
    Unknown,      //!< Unknown request action
};

void
from_json(const nlohmann::json &obj, RequestAction &action);

void
to_json(nlohmann::json &obj, const RequestAction &action);

//! A request to share a session key.
struct KeyRequest
{
    //! The type of request.
    RequestAction action;

    /// @brief The encryption algorithm of the session we want keys for.
    ///
    /// Always m.megolm.v1.aes-sha2.
    std::string algorithm;
    //! The room in which the session was created.
    std::string room_id;
    //! The curve25519 key of the session creator.
    std::string sender_key;
    //! The session_id of the outbound megolm session.
    std::string session_id;

    //! A unique identifier for this request.
    std::string request_id;
    //! The device requesting the keys.
    std::string requesting_device_id;

    friend void from_json(const nlohmann::json &obj, KeyRequest &event);
    friend void to_json(nlohmann::json &obj, const KeyRequest &event);
};

//! Content of the `m.key.verification.request` event
struct KeyVerificationRequest
{
    std::optional<std::string> body;
    //! The device ID which is initiating the request.
    std::string from_device;
    //! The device ID to which the key verification request is meant,used only for to-device
    //! verification
    std::optional<std::string> to;
    //! An opaque identifier for the verification request. Must be unique with respect to the
    //! devices involved.
    std::optional<std::string> transaction_id;
    //! must be `key.verification.request`, this field will be needed only in room verification
    std::optional<std::string> msgtype;
    //! The verification methods supported by the sender.
    std::vector<VerificationMethods> methods;
    //! The POSIX timestamp in milliseconds for when the request was made. If the request is in
    //! the future by more than 5 minutes or more than 10 minutes in the past, the message
    //! should be ignored by the receiver.
    std::optional<uint64_t> timestamp;

    friend void from_json(const nlohmann::json &obj, KeyVerificationRequest &event);
    friend void to_json(nlohmann::json &obj, const KeyVerificationRequest &event);
};

//! Content of the `m.key.verification.start` event
struct KeyVerificationStart
{
    //! The device ID which is initiating the process.
    std::string from_device;
    /// @brief An opaque identifier for the verification process.
    ///
    /// Must be unique with respect to the devices involved. Must be the same as the
    /// transaction_id given in the `m.key.verification.request` if this process is originating
    /// from a request.
    /// @note Used in verification via to_device messaging
    std::optional<std::string> transaction_id;
    //! The verification method to use. Must be 'm.sas.v1'
    VerificationMethods method = VerificationMethods::SASv1;
    /// @brief Optional method to use to verify the other user's key with.
    //
    // Applicable when the method chosen only verifies one user's key. This field will never be
    // present if the method verifies keys both ways.
    /// @note This appears to be unused in SAS verification
    std::optional<std::string> next_method;
    /// @brief The key agreement protocols the sending device understands.
    ///
    /// Must include at least curve25519.
    std::vector<std::string> key_agreement_protocols;
    //! The hash methods the sending device understands. Must include at least sha256.
    std::vector<std::string> hashes;
    /// @brief The message authentication codes that the sending device understands.
    ///
    /// Must include at least hkdf-hmac-sha256.
    std::vector<std::string> message_authentication_codes;
    /// @brief The SAS methods the sending device (and the sending device's user) understands.
    ///
    /// Must include at least decimal. Optionally can include emoji.
    ///
    /// One of:
    /// - `decimal`
    /// - `emoji`
    std::vector<SASMethods> short_authentication_string;
    /// @brief This is used for relating this message with previously sent
    /// `key.verification.request`
    ///
    /// @note Will be used only for room-verification msgs where this is used in place of
    /// transaction_id.
    common::Relations relations;

    friend void from_json(const nlohmann::json &obj, KeyVerificationStart &event);
    friend void to_json(nlohmann::json &obj, const KeyVerificationStart &event);
};

//! Implements the `m.key.verification.ready` event
struct KeyVerificationReady
{
    //! the deviceId of the device which send the `m.key.verification.request`
    std::string from_device;
    //! transactionId of the current flow
    std::optional<std::string> transaction_id;
    //! Sends the user the supported methods
    std::vector<VerificationMethods> methods;
    //! this is used for relating this message with previously sent
    //! key.verification.request will be used only for room-verification msgs where this
    //! is used in place of txnid
    common::Relations relations;

    friend void from_json(const nlohmann::json &obj, KeyVerificationReady &event);
    friend void to_json(nlohmann::json &obj, const KeyVerificationReady &event);
};

// ! Implements the `m.key.verification.done` event
struct KeyVerificationDone
{
    //! transactionId of the current flow
    std::optional<std::string> transaction_id;
    //! this is used for relating this message with previously sent key.verification.request
    //! will be used only for room-verification msgs where this is used in place of txnid
    common::Relations relations;

    friend void from_json(const nlohmann::json &obj, KeyVerificationDone &event);
    friend void to_json(nlohmann::json &obj, const KeyVerificationDone &event);
};

//! Implements the `m.key.verification.accept` event
struct KeyVerificationAccept
{
    //! when the method chosen only verifies one user's key. This field will never be present
    //! if the method verifies keys both ways.
    std::optional<std::string> transaction_id;
    //! The verification method to use. Must be 'm.sas.v1'
    VerificationMethods method = VerificationMethods::SASv1;
    //! The key agreement protocol the device is choosing to use, out of the options in the
    //! m.key.verification.start message.
    std::string key_agreement_protocol;
    //! The hash method the device is choosing to use, out of the options in the
    //! m.key.verification.start message.
    std::string hash;
    //! The message authentication code the device is choosing to use, out of the options in
    //! the m.key.verification.start message.
    std::string message_authentication_code;
    //! The SAS methods both devices involed in the verification process understand.  Must be
    //! a subset of the options in the m.key.verification.start message.
    //! One of: ["decimal", "emoji"]
    std::vector<SASMethods> short_authentication_string;
    //! The hash (encoded as unpadded base64) of the concatenation of the device's ephemeral
    //! public key (encoded as unpadded base64) and the canonical JSON representation of the
    //! m.key.verification.start message.
    std::string commitment;
    //! this is used for relating this message with previously sent key.verification.request
    //! will be used only for room-verification msgs where this is used in place of txnid
    common::Relations relations;

    friend void from_json(const nlohmann::json &obj, KeyVerificationAccept &event);
    friend void to_json(nlohmann::json &obj, const KeyVerificationAccept &event);
};

//! implementation of the `m.key.verification.cancel` event
struct KeyVerificationCancel
{
    //! The opaque identifier for the verification process/request.
    std::optional<std::string> transaction_id;
    //! A human readable description of the code. The client should only rely on this string
    //! if it does not understand the code.
    std::string reason;
    //! The error code for why the process/request was cancelled by the user. Error codes
    //! should use the Java package naming convention if not in the following list:
    //! m.user: The user cancelled the verification.
    //! m.timeout: The verification process timed out. Verification processes can define
    //!            their own timeout parameters.
    //! m.unknown_transaction: The device does not know about the given transaction ID.
    //! m.unknown_method: The device does not know how to handle the requested method.
    //!             This should be sent for m.key.verification.start messages and messages
    //!             defined by individual verification processes.
    //! m.unexpected_message: The device received an unexpected message. Typically raised
    //!             when one of the parties is handling the verification out of order.
    //! m.key_mismatch: The key was not verified.
    //! m.user_mismatch: The expected user did not match the user verified.
    //! m.invalid_message: The message received was invalid.
    //! m.accepted: A m.key.verification.request was accepted by a different device.
    //!             The device receiving this error can ignore the verification request.
    //! m.unknown_method: The devices are unable to agree on the key agreement,
    //!             hash, MAC, or SAS method.
    //! m.mismatched_commitment: The hash commitment did not match.
    //! m.mismatched_sas: The SAS did not match.
    //! Clients should be careful to avoid error loops. For example, if a device sends an
    //! incorrect message and the client returns m.invalid_message to which it gets an
    //! unexpected response with m.unexpected_message, the client should not respond
    //! again with m.unexpected_message to avoid the other device potentially sending
    //! another error response.
    std::string code;
    //! this is used for relating this message with previously sent key.verification.request
    //! will be used only for room-verification msgs where this is used in place of txnid
    common::Relations relations;

    friend void from_json(const nlohmann::json &obj, KeyVerificationCancel &event);
    friend void to_json(nlohmann::json &obj, const KeyVerificationCancel &event);
};

struct KeyVerificationKey
{
    //! An opaque identifier for the verification process. Must be the same as the one
    //! used for the m.key.verification.start message.
    std::optional<std::string> transaction_id;
    //! The device's ephemeral public key, encoded as unpadded base64.
    std::string key;
    //! this is used for relating this message with previously sent key.verification.request
    //! will be used only for room-verification msgs where this is used in place of txnid
    common::Relations relations;

    friend void from_json(const nlohmann::json &obj, KeyVerificationKey &event);
    friend void to_json(nlohmann::json &obj, const KeyVerificationKey &event);
};

struct KeyVerificationMac
{
    //! An opaque identifier for the verification process. Must be the same as the one
    //! used for the m.key.verification.start message.
    std::optional<std::string> transaction_id;
    //! A map of the key ID to the MAC of the key, using the algorithm in the
    //! verification process. The MAC is encoded as unpadded base64.
    std::map<std::string, std::string> mac;
    //! The MAC of the comma-separated, sorted, list of key IDs given in the mac
    //! property, encoded as unpadded base64.
    std::string keys;
    //! this is used for relating this message with previously sent key.verification.request
    //! will be used only for room-verification msgs where this is used in place of txnid
    common::Relations relations;

    friend void from_json(const nlohmann::json &obj, KeyVerificationMac &event);
    friend void to_json(nlohmann::json &obj, const KeyVerificationMac &event);
};

struct SecretRequest
{
    //! The type of request.
    RequestAction action;

    //! Required if action is request. The name of the secret that is being requested.
    std::string name;

    //! A unique identifier for this request.
    std::string request_id;
    //! The device requesting the keys.
    std::string requesting_device_id;

    friend void from_json(const nlohmann::json &obj, SecretRequest &event);
    friend void to_json(nlohmann::json &obj, const SecretRequest &event);
};

struct SecretSend
{
    //! Required. The contents of the secret.
    std::string secret;

    //! A unique identifier for this request.
    std::string request_id;

    friend void from_json(const nlohmann::json &obj, SecretSend &event);
    friend void to_json(nlohmann::json &obj, const SecretSend &event);
};

} // namespace msg
} // namespace events
} // namespace mtx
