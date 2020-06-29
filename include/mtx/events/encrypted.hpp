#pragma once

#include <nlohmann/json.hpp>

#include "mtx/events.hpp"
#include "mtx/events/common.hpp"

namespace mtx {
namespace events {
namespace msg {

enum class SASMethods
{
        Decimal,
        Emoji,
        Unsupported
};

void
from_json(const nlohmann::json &obj, SASMethods &method);

void
to_json(nlohmann::json &obj, const SASMethods &method);

//! TODO: Implement more if the verification methods ever change in KeyVerificationAccept
//! or otherwise
enum class VerificationMethods
{
        SASv1,
        Unsupported
};

void
from_json(const nlohmann::json &obj, VerificationMethods &method);

void
to_json(nlohmann::json &obj, const VerificationMethods &method);

struct OlmCipherContent
{
        std::string body;
        uint8_t type;
};

void
from_json(const nlohmann::json &obj, OlmCipherContent &event);

void
to_json(nlohmann::json &obj, const OlmCipherContent &event);

//! Content of the `m.room.encrypted` Olm event.
struct OlmEncrypted
{
        std::string algorithm;
        std::string sender_key;

        using RecipientKey = std::string;
        std::map<RecipientKey, OlmCipherContent> ciphertext;
};

void
from_json(const nlohmann::json &obj, OlmEncrypted &event);

void
to_json(nlohmann::json &obj, const OlmEncrypted &event);

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
        //! Relates to for rich replies
        common::ReplyRelatesTo relates_to;
};

void
from_json(const nlohmann::json &obj, Encrypted &event);

void
to_json(nlohmann::json &obj, const Encrypted &event);

//! Content of the `m.room_key` event.
struct RoomKey
{
        std::string algorithm;
        std::string room_id;
        std::string session_id;
        std::string session_key;
};

void
from_json(const nlohmann::json &obj, RoomKey &event);

void
to_json(nlohmann::json &obj, const RoomKey &event);

enum class RequestAction
{
        // request
        Request,
        // request_cancellation
        Cancellation,
        // not handled
        Unknown,
};

void
from_json(const nlohmann::json &obj, RequestAction &action);

void
to_json(nlohmann::json &obj, const RequestAction &action);

struct KeyRequest
{
        //! The type of request.
        RequestAction action;

        //! The encryption algorithm of the session we want keys for.
        //! Always m.megolm.v1.aes-sha2.
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
};

void
from_json(const nlohmann::json &obj, KeyRequest &event);

void
to_json(nlohmann::json &obj, const KeyRequest &event);

//! Content of the `m.key.verification.request` event
struct KeyVerificationRequest
{
        //! The device ID which is initiating the request.
        std::string from_device;
        //! An opaque identifier for the verification request. Must be unique with respect to the
        //! devices involved.
        std::string transaction_id;

        //! The verification methods supported by the sender.
        std::vector<VerificationMethods> methods;
        //! The POSIX timestamp in milliseconds for when the request was made. If the request is in
        //! the future by more than 5 minutes or more than 10 minutes in the past, the message
        //! should be ignored by the receiver.
        uint64_t timestamp;
};

void
from_json(const nlohmann::json &obj, KeyVerificationRequest &event);

void
to_json(nlohmann::json &obj, const KeyVerificationRequest &event);

//! Content of the `m.key.verification.start` event
struct KeyVerificationStart
{
        //! The device ID which is initiating the process.
        std::string from_device;
        //! An opaque identifier for the verification process. Must be unique with respect to the
        //! devices involved. Must be the same as the transaction_id given in the
        //! m.key.verification.request if this process is originating from a request.
        std::string transaction_id;
        //! The verification method to use. Must be 'm.sas.v1'
        VerificationMethods method = VerificationMethods::SASv1;
        //! Optional method to use to verify the other user's key with. Applicable when the method
        //! chosen only verifies one user's key. This field will never be present if the method
        //! verifies keys both ways.
        //! NOTE: This appears to be unused in SAS verification
        std::optional<std::string> next_method;
        //! The key agreement protocols the sending device understands.
        //! Must include at least curve25519.
        std::vector<std::string> key_agreement_protocols;
        //! The hash methods the sending device understands. Must include at least sha256.
        std::vector<std::string> hashes;
        //! The message authentication codes that the sending device understands.
        //! Must include at least hkdf-hmac-sha256.
        std::vector<std::string> message_authentication_codes;
        //! The SAS methods the sending device (and the sending device's user) understands.
        //! Must include at least decimal. Optionally can include emoji.
        //! One of: ["decimal", "emoji"]
        std::vector<SASMethods> short_authentication_string;
};

void
from_json(const nlohmann::json &obj, KeyVerificationStart &event);

void
to_json(nlohmann::json &obj, const KeyVerificationStart &event);

//! Implements the `m.key.verification.accept` event
struct KeyVerificationAccept
{
        //! when the method chosen only verifies one user's key. This field will never be present
        //! if the method verifies keys both ways.
        std::string transaction_id;
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
};

void
from_json(const nlohmann::json &obj, KeyVerificationAccept &event);

void
to_json(nlohmann::json &obj, const KeyVerificationAccept &event);

//! implementation of the `m.key.verification.cancel` event
struct KeyVerificationCancel
{
        //! The opaque identifier for the verification process/request.
        std::string transaction_id;
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
};

void
from_json(const nlohmann::json &obj, KeyVerificationCancel &event);

void
to_json(nlohmann::json &obj, const KeyVerificationCancel &event);

struct KeyVerificationKey
{
        //! An opaque identifier for the verification process. Must be the same as the one
        //! used for the m.key.verification.start message.
        std::string transaction_id;
        //! The device's ephemeral public key, encoded as unpadded base64.
        std::string key;
};

void
from_json(const nlohmann::json &obj, KeyVerificationKey &event);

void
to_json(nlohmann::json &obj, const KeyVerificationKey &event);

struct KeyVerificationMac
{
        //! An opaque identifier for the verification process. Must be the same as the one
        //! used for the m.key.verification.start message.
        std::string transaction_id;
        //! A map of the key ID to the MAC of the key, using the algorithm in the
        //! verification process. The MAC is encoded as unpadded base64.
        std::map<std::string, std::string> mac;
        //! The MAC of the comma-separated, sorted, list of key IDs given in the mac
        //! property, encoded as unpadded base64.
        std::string keys;
};

void
from_json(const nlohmann::json &obj, KeyVerificationMac &event);

void
to_json(nlohmann::json &obj, const KeyVerificationMac &event);

} // namespace msg
struct DeviceEventVisitor
{
        nlohmann::json operator()(const DeviceEvent<mtx::events::msg::RoomKey> &roomKey)
        {
                json j;
                mtx::events::to_json(j, roomKey);
                return j;
        }
        nlohmann::json operator()(const DeviceEvent<mtx::events::msg::KeyRequest> &keyReq)
        {
                json j;
                mtx::events::to_json(j, keyReq);
                return j;
        }
        nlohmann::json operator()(const DeviceEvent<mtx::events::msg::OlmEncrypted> &olmEnc)
        {
                json j;
                mtx::events::to_json(j, olmEnc);
                return j;
        }
        nlohmann::json operator()(const DeviceEvent<mtx::events::msg::Encrypted> &enc)
        {
                json j;
                mtx::events::to_json(j, enc);
                return j;
        }
        nlohmann::json operator()(
          const DeviceEvent<mtx::events::msg::KeyVerificationRequest> &keyVerificationRequest)
        {
                json j;
                mtx::events::to_json(j, keyVerificationRequest);
                return j;
        }
        nlohmann::json operator()(
          const DeviceEvent<mtx::events::msg::KeyVerificationAccept> &keyVerificationAccept)
        {
                json j;
                mtx::events::to_json(j, keyVerificationAccept);
                return j;
        }
        nlohmann::json operator()(
          const DeviceEvent<mtx::events::msg::KeyVerificationStart> &keyVerificationStart)
        {
                json j;
                mtx::events::to_json(j, keyVerificationStart);
                return j;
        }
        nlohmann::json operator()(
          const DeviceEvent<mtx::events::msg::KeyVerificationCancel> &KeyVerificationCancel)
        {
                json j;
                mtx::events::to_json(j, KeyVerificationCancel);
                return j;
        }
        nlohmann::json operator()(
          const DeviceEvent<mtx::events::msg::KeyVerificationKey> &keyVerificationKey)
        {
                json j;
                mtx::events::to_json(j, keyVerificationKey);
                return j;
        }
        nlohmann::json operator()(
          const DeviceEvent<mtx::events::msg::KeyVerificationMac> &keyVerificationMac)
        {
                json j;
                mtx::events::to_json(j, keyVerificationMac);
                return j;
        }
};
} // namespace events
} // namespace mtx
