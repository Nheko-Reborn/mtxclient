#pragma once

/// @file
/// @brief Holds most of the crypto functions and errors as well as a Client, which does the Olm
/// account bookkeeping for you.

#include <exception>
#include <memory>
#include <new>

#if __has_include(<nlohmann/json_fwd.hpp>)
#include <nlohmann/json_fwd.hpp>
#else
#include <nlohmann/json.hpp>
#endif

#include <mtx/identifiers.hpp>
#include <mtx/requests.hpp>

#include <olm/olm.h>
#include <olm/sas.h>

#include "mtxclient/crypto/objects.hpp"
#include "mtxclient/crypto/types.hpp"
#include "mtxclient/crypto/utils.hpp"

namespace mtx {
//! Cryptography related types
namespace crypto {

//! Data representation used to interact with libolm.
using BinaryBuf = std::vector<uint8_t>;

enum class OlmErrorCode
{
    UNKNOWN_ERROR = -1,
    SUCCESS,
    NOT_ENOUGH_RANDOM,
    OUTPUT_BUFFER_TOO_SMALL,
    BAD_MESSAGE_VERSION,
    BAD_MESSAGE_FORMAT,
    BAD_MESSAGE_MAC,
    BAD_MESSAGE_KEY_ID,
    INVALID_BASE64,
    BAD_ACCOUNT_KEY,
    UNKNOWN_PICKLE_VERSION,
    CORRUPTED_PICKLE,
    BAD_SESSION_KEY,
    UNKNOWN_MESSAGE_INDEX,
    BAD_LEGACY_ACCOUNT_PICKLE,
    BAD_SIGNATURE,
    OLM_INPUT_BUFFER_TOO_SMALL,
    OLM_SAS_THEIR_KEY_NOT_SET
};

//! Errors returned by the olm library
class olm_exception : public std::exception
{
public:
    olm_exception(std::string func, OlmSession *s)
      : olm_exception(std::move(func), std::string(olm_session_last_error(s)))
    {}

    olm_exception(std::string func, OlmAccount *acc)
      : olm_exception(std::move(func),
                      std::string(acc ? olm_account_last_error(acc) : "account == nullptr"))
    {}

    olm_exception(std::string func, OlmUtility *util)
      : olm_exception(std::move(func), std::string(olm_utility_last_error(util)))
    {}

    olm_exception(std::string func, OlmPkDecryption *s)
      : olm_exception(std::move(func), std::string(olm_pk_decryption_last_error(s)))
    {}

    olm_exception(std::string func, OlmPkEncryption *s)
      : olm_exception(std::move(func), std::string(olm_pk_encryption_last_error(s)))
    {}

    olm_exception(std::string func, OlmPkSigning *s)
      : olm_exception(std::move(func), std::string(olm_pk_signing_last_error(s)))
    {}

    olm_exception(std::string func, OlmOutboundGroupSession *s)
      : olm_exception(
          std::move(func),
          std::string(s ? olm_outbound_group_session_last_error(s) : "session == nullptr"))
    {}

    olm_exception(std::string func, OlmInboundGroupSession *s)
      : olm_exception(
          std::move(func),
          std::string(s ? olm_inbound_group_session_last_error(s) : "session == nullptr"))
    {}

    olm_exception(std::string func, OlmSAS *s)
      : olm_exception(std::move(func), std::string(olm_sas_last_error(s)))
    {}

    //! Returns a description of the olm error.
    const char *what() const noexcept override { return msg_.c_str(); }

    //! Returns an error code reconstructed from the error string returned by olm
    OlmErrorCode error_code() const noexcept { return ec_; }

private:
    olm_exception(std::string &&func, std::string error_string)
      : msg_(func + ": " + error_string)
      , ec_(ec_from_string(error_string))
    {}

    OlmErrorCode ec_from_string(std::string_view);

    std::string msg_;
    OlmErrorCode ec_ = OlmErrorCode::UNKNOWN_ERROR;
};

//! Serialize olm objects into strings encrypted using key to persist them on non volatile storage.
template<class T>
std::string
pickle(typename T::olm_type *object, const std::string &key)
{
    auto tmp       = create_buffer(T::pickle_length(object));
    const auto ret = T::pickle(object, key.data(), key.size(), tmp.data(), tmp.size());

    if (ret == olm_error())
        throw olm_exception("pickle", object);

    return std::string((char *)tmp.data(), tmp.size());
}

//! Deserialize olm objects from strings encrypted using key previously persisted on non volatile
//! storage.
template<class T>
std::unique_ptr<typename T::olm_type, OlmDeleter>
unpickle(const std::string &pickled, const std::string &key)
{
    auto object = create_olm_object<T>();

    auto ret =
      T::unpickle(object.get(), key.data(), key.size(), (void *)pickled.data(), pickled.size());

    if (ret == olm_error())
        throw olm_exception("unpickle", object.get());

    return object;
}

//! Return value from decrypting a group message.
struct GroupPlaintext
{
    //! The plain text, which was decrypted.
    BinaryBuf data;
    //! The message index used for this message.
    uint32_t message_index;
};

//! Helper to generate Short Authentication Strings (SAS)
struct SAS
{
    //! Create a new SAS object.
    SAS();
    //! Query the public key generated for this object.
    std::string public_key();
    //! Set the key of the other party being verified.
    void set_their_key(const std::string &their_public_key);
    /// @brief Returns 3 integers ranging from 1000 to 9191, to be used only after
    /// using `set_their_key`
    ///
    /// These are meant to be compared by the users verifying each other.
    std::vector<int> generate_bytes_decimal(const std::string &info);
    /// @brief Returns 7 integers in the range from 0 to 63, to be used only after using
    /// `set_their_key`
    ///
    /// Map these numpers to one of the 64 emoji from the specification and let the user compare
    /// them.
    std::vector<int> generate_bytes_emoji(const std::string &info);
    //! Calculate MACs after verification to verify keys.
    std::string calculate_mac(const std::string &input_data, const std::string &info);

private:
    SASPtr sas;
};

//! Helper to sign arbitrary messages using an ed25519 key
struct PkSigning
{
    //! Construct from base64 key
    static PkSigning from_seed(const std::string &seed);
    //! construct a new random key
    static PkSigning new_key();

    //! sign an arbitrary message
    std::string sign(const std::string &message);

    //! base64 public key
    std::string public_key() const { return public_key_; }
    //! base64 private key (seed)
    std::string seed() const { return seed_; }

private:
    PkSigning() {}
    std::unique_ptr<OlmPkSigning, OlmDeleter> signing;
    std::string public_key_; // base64
    std::string seed_;       // base64
};

//! Client for all the cryptography related functionality like olm accounts, session keys
//! encryption, signing and a few more things.
class OlmClient : public std::enable_shared_from_this<OlmClient>
{
public:
    OlmClient() = default;
    //! Initialize a crypto client for the specified device of the specified user.
    OlmClient(std::string user_id, std::string device_id)
      : user_id_(std::move(user_id))
      , device_id_(std::move(device_id))
    {}

    //! Base64 encoded string
    using Base64String = std::string;
    //! A signed set of one time keys indexed by `<algorithm>:<key_id>`.
    using SignedOneTimeKeys = std::map<std::string, requests::SignedOneTimeKey>;

    //! Data needed for bootstrapping crosssigning
    struct CrossSigningSetup
    {
        //! The public key objects, signed and ready for upload.
        CrossSigningKeys master_key, user_signing_key, self_signing_key;
        //! The private keys to store in SSSS
        std::string private_master_key, private_user_signing_key,
          private_self_signing_key; // base64
    };

    //! Data needed to setup the online key backup
    struct OnlineKeyBackupSetup
    {
        //! private key to decrypt sessions with.
        mtx::crypto::BinaryBuf privateKey;
        //! The backup version data including auth data to be sent to the server.
        mtx::responses::backup::BackupVersion backupVersion;
    };
    //! Data needed to setup SSSS
    struct SSSSSetup
    {
        //! Key to encrypt/decrypt secrets with.
        mtx::crypto::BinaryBuf privateKey;
        //! The key description to be stored in account data.
        mtx::secret_storage::AesHmacSha2KeyDescription keyDescription;
        //! The name of this key.
        std::string key_name;
    };

    //! Set the id of this device.
    void set_device_id(std::string device_id) { device_id_ = std::move(device_id); }
    //! Set the id of this user.
    void set_user_id(std::string user_id) { user_id_ = std::move(user_id); }

    //! Sign the given message.
    Base64String sign_message(const std::string &msg) const;

    //! Create a new olm Account. Must be called before any other operation.
    void create_new_account();
    //! Create a new olm utility object.
    void create_new_utility() { utility_ = create_olm_object<UtilityObject>(); }

    /// @brief Restore the olm account from a pickled string encrypted by `key`
    /// @see load
    void restore_account(const std::string &saved_data, const std::string &key);

    //! Retrieve the json representation of the identity keys for the given account.
    IdentityKeys identity_keys() const;
    //! Sign the identity keys.
    //! The result should be used as part of the /keys/upload/ request.
    Base64String sign_identity_keys();

    //! Generate a number of one time keys.
    std::size_t generate_one_time_keys(std::size_t nkeys, bool generate_fallback = false);
    //! Retrieve the json representation of the one time keys for the given account.
    OneTimeKeys one_time_keys();
    //! Retrieve the json representation of the unpublished fallback one time keys for the given
    //! account.
    OneTimeKeys unpublished_fallback_keys();
    //! Sign the given one time keys and encode it to base64.
    Base64String sign_one_time_key(const Base64String &encoded_key, bool fallback = false);
    //! Sign one_time_keys and generate the appropriate structure for the /keys/upload request.
    SignedOneTimeKeys sign_one_time_keys(const OneTimeKeys &keys, bool fallback = false);
    //! Generate the json structure for the signed one time key.
    requests::SignedOneTimeKey signed_one_time_key(const std::string &key,
                                                   const std::string &signature,
                                                   bool fallback = false);

    //! Marks the current set of one time keys as being published.
    void mark_keys_as_published() { olm_account_mark_keys_as_published(account_.get()); }
    //! Forgets an old fallback key. Call this when you are sure the old key is no longer in use,
    //! i.e. 5 minutes after publishing a new one.
    void forget_old_fallback_key() { olm_account_forget_old_fallback_key(account_.get()); }

    //! Prepare request for the /keys/upload endpoint by signing identity & one time keys.
    mtx::requests::UploadKeys create_upload_keys_request(const OneTimeKeys &keys,
                                                         const OneTimeKeys &fallback_keys);
    //! Prepare an empty /keys/upload request.
    mtx::requests::UploadKeys create_upload_keys_request();

    //! Create the cross-signing keys (including signatures). Needs to be uploaded to the server
    //! after this.
    std::optional<CrossSigningSetup> create_crosssigning_keys();

    //! Create a new online key backup. Needs to be uploaded to the server after this.
    std::optional<OnlineKeyBackupSetup> create_online_key_backup(const std::string &masterKey);
    //! Create a new SSSS storage key. Should be uploaded to account_data. The password is optional.
    static std::optional<SSSSSetup> create_ssss_key(const std::string &password = "");

    //! Decrypt a message using megolm.
    GroupPlaintext decrypt_group_message(OlmInboundGroupSession *session,
                                         const std::string &message,
                                         uint32_t message_index = 0);
    //! Encrypt a message using megolm.
    BinaryBuf encrypt_group_message(OlmOutboundGroupSession *session, const std::string &plaintext);
    //! Encrypt a message using olm.
    BinaryBuf encrypt_message(OlmSession *session, const std::string &msg);
    //! Decrypt a message using olm.
    BinaryBuf decrypt_message(OlmSession *session, std::size_t msg_type, const std::string &msg);

    /// @brief Create an outbound megolm session.
    /// @sa init_inbound_group_session(const std::string&), import_inbound_group_session()
    OutboundGroupSessionPtr init_outbound_group_session();
    /// @brief Initialize an inbound group session from a shared session key (an m.room_key
    /// event).
    /// @sa init_inbound_group_session(), import_inbound_group_session()
    InboundGroupSessionPtr init_inbound_group_session(const std::string &session_key);
    /// @brief Initialize an inbound group session from a forwarded session key (an
    /// m.forwarded_room_key event).
    /// @sa init_inbound_group_session(const std::string&), init_inbound_group_session()
    InboundGroupSessionPtr import_inbound_group_session(const std::string &session_key);

    /// @brief create an outbound session to encrypt to device messages.
    /// @param identity_key The curve25519 key of the other party.
    /// @param one_time_key The claimed one time key of the other party.
    OlmSessionPtr create_outbound_session(const std::string &identity_key,
                                          const std::string &one_time_key);
    /// @brief Creates an inbound session from an inbound message. DON'T USE THIS, use
    /// create_inbound_session_from() instead.
    /// @sa create_inbound_session_from()
    OlmSessionPtr create_inbound_session(const BinaryBuf &one_time_key_message);
    /// @brief Creates an inbound session from an inbound message. DON'T USE THIS, use
    /// create_inbound_session_from() instead.
    /// @sa create_inbound_session_from()
    OlmSessionPtr create_inbound_session(const std::string &one_time_key_message);

    /// @brief Create an inbound olm session from the other users message and identity key
    /// @sa create_inbound_session_from(const std::string&, const std::string&),
    /// create_outbound_session(), create_inbound_session()
    OlmSessionPtr create_inbound_session_from(const std::string &their_curve25519,
                                              const BinaryBuf &one_time_key_message);
    /// @brief Create an inbound olm session from the other users message and identity key
    /// @sa create_inbound_session_from(const std::string&, const BinaryBuf&),
    /// create_outbound_session(), create_inbound_session()
    OlmSessionPtr create_inbound_session_from(const std::string &their_curve25519,
                                              const std::string &one_time_key_message);

    //! Create the content for an m.room.encrypted event.
    //! algorithm: m.olm.v1.curve25519-aes-sha2
    nlohmann::json create_olm_encrypted_content(OlmSession *session,
                                                nlohmann::json event,
                                                const UserId &recipient,
                                                const std::string &recipient_ed25519_key,
                                                const std::string &recipient_curve25519_key);

    //! store the account in a pickled string encrypted by `key`
    std::string save(const std::string &key);
    /// @brief Restore the account from a pickled string encrypted by `key`
    /// @see restore_account
    void load(const std::string &data, const std::string &key)
    {
        account_ = unpickle<AccountObject>(data, key);
    }

    //! Access the olm account directly.
    OlmAccount *account() { return account_.get(); }
    //! Access the olm utility object directly.
    OlmUtility *utility() { return utility_.get(); }

    //! SAS related stuff
    //! this creates a unique pointer of struct SAS
    std::unique_ptr<SAS> sas_init();

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

// @brief Retrieve the session key from an *inbound* megolm session with a specific minimum index.
//
// Use -1 to use the smallest index possible
std::string
export_session(OlmInboundGroupSession *s, uint32_t at_index);

//! Create an *inbound* megolm session from an exported session key.
InboundGroupSessionPtr
import_session(const std::string &session_key);

/// Checks if an inbound session matches a pre key message.
///
/// Use matches_inbound_session_from() instead.
bool
matches_inbound_session(OlmSession *session, const std::string &one_time_key_message);

//! Checks if an inbound session matches a pre key message
bool
matches_inbound_session_from(OlmSession *session,
                             const std::string &id_key,
                             const std::string &one_time_key_message);

//! Encrypt the exported sessions according to the export format from the spec.
std::string
encrypt_exported_sessions(const mtx::crypto::ExportedSessionKeys &keys, const std::string &pass);

//! Decrypt the exported sessions according to the export format from the spec.
mtx::crypto::ExportedSessionKeys
decrypt_exported_sessions(const std::string &data, const std::string &pass);

//! Verify a signature object as obtained from the response of /keys/query endpoint
bool
verify_identity_signature(const DeviceKeys &device_keys,
                          const DeviceId &device_id,
                          const UserId &user_id);
//! Verify an ed25519 signature.
bool
ed25519_verify_signature(std::string signing_key, nlohmann::json obj, std::string signature);

} // namespace crypto
} // namespace mtx
