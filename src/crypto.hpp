#pragma once

#include <exception>
#include <memory>
#include <new>

#include <json.hpp>
#include <sodium.h>

#include <mtx/identifiers.hpp>
#include <mtx/requests.hpp>

#include <olm/account.hh>
#include <olm/error.h>
#include <olm/session.hh>

#include <olm/olm.h>

namespace mtx {
namespace client {
namespace crypto {

static constexpr const char *ED25519    = "ed25519";
static constexpr const char *CURVE25519 = "curve25519";

//! Data representation used to interact with libolm.
using BinaryBuf = std::vector<uint8_t>;

struct IdentityKeys
{
        std::string curve25519;
        std::string ed25519;
};

inline void
to_json(nlohmann::json &obj, const IdentityKeys &keys)
{
        obj[ED25519]    = keys.ed25519;
        obj[CURVE25519] = keys.curve25519;
}

inline void
from_json(const nlohmann::json &obj, IdentityKeys &keys)
{
        keys.ed25519    = obj.at(ED25519).get<std::string>();
        keys.curve25519 = obj.at(CURVE25519).get<std::string>();
}

struct OneTimeKeys
{
        using KeyId      = std::string;
        using EncodedKey = std::string;

        std::map<KeyId, EncodedKey> curve25519;
};

inline void
to_json(nlohmann::json &obj, const OneTimeKeys &keys)
{
        obj["curve25519"] = keys.curve25519;
}

inline void
from_json(const nlohmann::json &obj, OneTimeKeys &keys)
{
        keys.curve25519 = obj.at("curve25519").get<std::map<std::string, std::string>>();
}

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

        olm_exception(std::string msg)
          : msg_(msg)
        {}

        virtual const char *what() const throw() { return msg_.c_str(); }

private:
        std::string msg_;
};

//! Create a uint8_t buffer which is initialized with random bytes.
template<class T = BinaryBuf>
std::unique_ptr<T>
create_buffer(std::size_t nbytes)
{
        auto buf = std::make_unique<T>(nbytes);
        randombytes_buf(buf->data(), buf->size());

        return buf;
}

struct OlmDeleter
{
        void operator()(OlmAccount *ptr) { operator delete(ptr, olm_account_size()); }
        void operator()(OlmSession *ptr) { operator delete(ptr, olm_session_size()); }
        void operator()(OlmUtility *ptr) { operator delete(ptr, olm_utility_size()); }
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
        std::unique_ptr<BinaryBuf> sign_message(const std::string &msg);

        //! Create a new olm Account. Must be called before any other operation.
        void create_new_account();
        void create_new_utility();
        std::unique_ptr<OlmSession, OlmDeleter> create_new_session();

        //! Retrieve the json representation of the identity keys for the given account.
        IdentityKeys identity_keys();
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

        //! Prepare request for the /keys/upload endpoint by signing identity & one time keys.
        mtx::requests::UploadKeys create_upload_keys_request(const OneTimeKeys &keys);
        mtx::requests::UploadKeys create_upload_keys_request();

        //! Create an outbount megolm session.
        std::unique_ptr<OlmSession, OlmDeleter> create_outbound_group_session(
          const std::string &peer_identity_key,
          const std::string &peer_one_time_key);

        OlmAccount *account() { return account_.get(); }
        OlmUtility *utility() { return utility_.get(); }

private:
        std::string user_id_;
        std::string device_id_;

        std::unique_ptr<OlmAccount, OlmDeleter> account_;
        std::unique_ptr<OlmUtility, OlmDeleter> utility_;
};

std::string
encode_base64(const uint8_t *data, std::size_t len);

//! Decode the given base64 string
std::unique_ptr<BinaryBuf>
decode_base64(const std::string &data);

//! Convert the given json struct to an uint8_t buffer.
std::unique_ptr<BinaryBuf>
json_to_buffer(const nlohmann::json &obj);

//! Convert the given string to an uint8_t buffer.
std::unique_ptr<BinaryBuf>
str_to_buffer(const std::string &data);

} // namespace crypto
} // namespace client
} // namespace mtx
