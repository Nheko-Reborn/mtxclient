#pragma once

#include <exception>
#include <memory>
#include <new>

#include <nlohmann/json.hpp>
#include <sodium.h>

#include <mtx/identifiers.hpp>
#include <mtx/requests.hpp>

#include <olm/olm.h>

#include "mtxclient/crypto/objects.hpp"
#include "mtxclient/crypto/types.hpp"

namespace mtx {
namespace crypto {

//! Data representation used to interact with libolm.
using BinaryBuf = std::vector<uint8_t>;

class olm_exception : public std::exception
{
public:
        olm_exception(std::string func, OlmSession *s)
          : msg_(func + ": " + std::string(olm_session_last_error(s)))
        {}

        olm_exception(std::string func, OlmAccount *acc)
          : msg_(func + ": " + std::string(olm_account_last_error(acc)))
        {}

        olm_exception(std::string func, OlmUtility *util)
          : msg_(func + ": " + std::string(olm_utility_last_error(util)))
        {}

        olm_exception(std::string func, OlmOutboundGroupSession *s)
          : msg_(func + ": " + std::string(olm_outbound_group_session_last_error(s)))
        {}

        olm_exception(std::string func, OlmInboundGroupSession *s)
          : msg_(func + ": " + std::string(olm_inbound_group_session_last_error(s)))
        {}

        olm_exception(std::string msg)
          : msg_(msg)
        {}

        virtual const char *what() const throw() { return msg_.c_str(); }

private:
        std::string msg_;
};

class sodium_exception : public std::exception
{
public:
        sodium_exception(std::string func, const char *msg)
          : msg_(func + ": " + std::string(msg))
        {}

        virtual const char *what() const throw() { return msg_.c_str(); }

private:
        std::string msg_;
};

//! Create a uint8_t buffer which is initialized with random bytes.
inline BinaryBuf
create_buffer(std::size_t nbytes)
{
        auto buf = BinaryBuf(nbytes);
        randombytes_buf(buf.data(), buf.size());

        return buf;
}

template<class T>
std::string
pickle(typename T::olm_type *object, const std::string &key)
{
        auto tmp      = create_buffer(T::pickle_length(object));
        const int ret = T::pickle(object, key.data(), key.size(), tmp.data(), tmp.size());

        if (ret == -1)
                throw olm_exception("pickle", object);

        return std::string((char *)tmp.data(), tmp.size());
}

template<class T>
std::unique_ptr<typename T::olm_type, OlmDeleter>
unpickle(const std::string &pickled, const std::string &key)
{
        auto object = create_olm_object<T>();

        const int ret =
          T::unpickle(object.get(), key.data(), key.size(), (void *)pickled.data(), pickled.size());

        if (ret == -1)
                throw olm_exception("unpickle", object.get());

        return std::move(object);
}

using OlmSessionPtr           = std::unique_ptr<OlmSession, OlmDeleter>;
using OutboundGroupSessionPtr = std::unique_ptr<OlmOutboundGroupSession, OlmDeleter>;
using InboundGroupSessionPtr  = std::unique_ptr<OlmInboundGroupSession, OlmDeleter>;

struct GroupPlaintext
{
        BinaryBuf data;
        uint32_t message_index;
};

class OlmClient : public std::enable_shared_from_this<OlmClient>
{
public:
        OlmClient() = default;
        OlmClient(std::string user_id, std::string device_id)
          : user_id_(std::move(user_id))
          , device_id_(std::move(device_id))
        {}

        using Base64String      = std::string;
        using SignedOneTimeKeys = std::map<std::string, json>;

        void set_device_id(std::string device_id) { device_id_ = std::move(device_id); }
        void set_user_id(std::string user_id) { user_id_ = std::move(user_id); }

        //! Sign the given message.
        Base64String sign_message(const std::string &msg) const;

        //! Create a new olm Account. Must be called before any other operation.
        void create_new_account();
        void create_new_utility() { utility_ = create_olm_object<UtilityObject>(); }

        void restore_account(const std::string &saved_data, const std::string &key);

        //! Retrieve the json representation of the identity keys for the given account.
        IdentityKeys identity_keys() const;
        //! Sign the identity keys.
        //! The result should be used as part of the /keys/upload/ request.
        Base64String sign_identity_keys();

        //! Generate a number of one time keys.
        std::size_t generate_one_time_keys(std::size_t nkeys);
        //! Retrieve the json representation of the one time keys for the given account.
        OneTimeKeys one_time_keys();
        //! Sign the given one time keys and encode it to base64.
        Base64String sign_one_time_key(const Base64String &encoded_key);
        //! Sign one_time_keys and generate the appropriate structure for the /keys/upload request.
        SignedOneTimeKeys sign_one_time_keys(const OneTimeKeys &keys);
        //! Generate the json structure for the signed one time key.
        json signed_one_time_key_json(const std::string &key, const std::string &signature);

        //! Marks the current set of one time keys as being published.
        void mark_keys_as_published() { olm_account_mark_keys_as_published(account_.get()); }

        //! Prepare request for the /keys/upload endpoint by signing identity & one time keys.
        mtx::requests::UploadKeys create_upload_keys_request(const OneTimeKeys &keys);
        mtx::requests::UploadKeys create_upload_keys_request();

        //! Decrypt a message using megolm.
        GroupPlaintext decrypt_group_message(OlmInboundGroupSession *session,
                                             const std::string &message,
                                             uint32_t message_index = 0);
        //! Encrypt a message using megolm.
        BinaryBuf encrypt_group_message(OlmOutboundGroupSession *session,
                                        const std::string &plaintext);
        //! Encrypt a message using olm.
        BinaryBuf encrypt_message(OlmSession *session, const std::string &msg);
        //! Decrypt a message using olm.
        BinaryBuf decrypt_message(OlmSession *session,
                                  std::size_t msg_type,
                                  const std::string &msg);

        //! Create an outbount megolm session.
        OutboundGroupSessionPtr init_outbound_group_session();
        InboundGroupSessionPtr init_inbound_group_session(const std::string &session_key);
        OlmSessionPtr create_outbound_session(const std::string &identity_key,
                                              const std::string &one_time_key);
        OlmSessionPtr create_inbound_session(const BinaryBuf &one_time_key_message);
        OlmSessionPtr create_inbound_session(const std::string &one_time_key_message);

        OlmSessionPtr create_inbound_session_from(const std::string &their_curve25519,
                                                  const BinaryBuf &one_time_key_message);
        OlmSessionPtr create_inbound_session_from(const std::string &their_curve25519,
                                                  const std::string &one_time_key_message);

        //! The `m.room_key` event is used to share the session_id & session_key
        //! of an outbound megolm session.
        nlohmann::json create_room_key_event(const UserId &user_id,
                                             const std::string &ed25519_device_key,
                                             const nlohmann::json &content) const noexcept;

        //! Create the content for an m.room.encrypted event.
        //! algorithm: m.olm.v1.curve25519-aes-sha2
        nlohmann::json create_olm_encrypted_content(OlmSession *session,
                                                    const std::string &room_key_event,
                                                    const std::string &recipient_key);

        std::string save(const std::string &key);
        void load(const std::string &data, const std::string &key)
        {
                account_ = unpickle<AccountObject>(data, key);
        }

        OlmAccount *account() { return account_.get(); }
        OlmUtility *utility() { return utility_.get(); }

private:
        std::string user_id_;
        std::string device_id_;

        std::unique_ptr<OlmAccount, OlmDeleter> account_;
        std::unique_ptr<OlmUtility, OlmDeleter> utility_;
};

//! Retrieve the session id for an Olm session.
std::string
session_id(OlmSession *s);

//! Retrieve the session id.
std::string
session_id(OlmOutboundGroupSession *s);

//! Retrieve the session key from an *outbound* megolm session.
std::string
session_key(OlmOutboundGroupSession *s);

//! Retrieve the session key from an *inbound* megolm session.
std::string
export_session(OlmInboundGroupSession *s);

//! Create an *inbound* megolm session from an exported session key.
InboundGroupSessionPtr
import_session(const std::string &session_key);

bool
matches_inbound_session(OlmSession *session, const std::string &one_time_key_message);

bool
matches_inbound_session_from(OlmSession *session,
                             const std::string &id_key,
                             const std::string &one_time_key_message);

std::string
encrypt_exported_sessions(const mtx::crypto::ExportedSessionKeys &keys, std::string pass);

mtx::crypto::ExportedSessionKeys
decrypt_exported_sessions(const std::string &data, std::string pass);

std::string
base642bin(const std::string &b64);

std::string
bin2base64(const std::string &b64);

BinaryBuf
derive_key(const std::string &pass, const BinaryBuf &salt);

//! Verify a signature object as obtained from the response of /keys/query endpoint
bool
verify_identity_signature(nlohmann::json obj, const DeviceId &device_id, const UserId &user_id);

} // namespace crypto
} // namespace mtx
