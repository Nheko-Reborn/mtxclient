#pragma once

#include <exception>
#include <memory>

#include <json.hpp>
#include <sodium.h>

#include <mtx/identifiers.hpp>
#include <mtx/requests.hpp>

#include <olm/account.hh>
#include <olm/error.h>
#include <olm/session.hh>

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
        olm_exception(std::string msg, OlmErrorCode errcode)
          : errcode_(errcode)
          , msg_(msg + ": " + std::string(_olm_error_to_string(errcode)))
        {}

        olm_exception(std::string msg)
          : msg_(msg)
        {}

        OlmErrorCode get_errcode() const { return errcode_; }
        const char *get_error() const { return _olm_error_to_string(errcode_); }

        virtual const char *what() const throw() { return msg_.c_str(); }

private:
        OlmErrorCode errcode_;
        std::string msg_;
};

//! Create a new olm Account.
std::shared_ptr<olm::Account>
olm_new_account();

//! Retrieve the json representation of the identity keys for the given account.
IdentityKeys
identity_keys(std::shared_ptr<olm::Account> user);

//! Generate a number of one time keys.
std::size_t
generate_one_time_keys(std::shared_ptr<olm::Account> account, std::size_t number_of_keys);

//! Retrieve the json representation of the one time keys for the given account.
nlohmann::json
one_time_keys(std::shared_ptr<olm::Account> user);

//! Create a uint8_t buffer which is initialized with random bytes.
template<class T = BinaryBuf>
std::unique_ptr<T>
create_buffer(std::size_t nbytes)
{
        auto buf = std::make_unique<T>(nbytes);
        randombytes_buf(buf->data(), buf->size());

        return buf;
}

//! Sign the given one time keys and encode it to base64.
std::string
sign_one_time_key(std::shared_ptr<olm::Account> account, const std::string &key);

//! Sign the identity keys. The result should be used as part of the /keys/upload/ request.
std::string
sign_identity_keys(std::shared_ptr<olm::Account> account,
                   const IdentityKeys &keys,
                   const mtx::identifiers::User &user_id,
                   const std::string &device_id);

//! Sign the given message.
std::unique_ptr<BinaryBuf>
sign_message(std::shared_ptr<olm::Account> account, const std::string &msg);

//! Generate the json structure for the signed one time key.
nlohmann::json
signed_one_time_key_json(const mtx::identifiers::User &user_id,
                         const std::string &device_id,
                         const std::string &key,
                         const std::string &signature);

//! Sign one_time_keys and generate the appropriate structure for the /keys/upload request.
std::map<std::string, nlohmann::json>
sign_one_time_keys(std::shared_ptr<olm::Account> account,
                   const mtx::client::crypto::OneTimeKeys &keys,
                   const mtx::identifiers::User &user_id,
                   const std::string &device_id);

//! Prepare request for the /keys/upload endpoint by signing identity & one time keys.
mtx::requests::UploadKeys
create_upload_keys_request(std::shared_ptr<olm::Account> account,
                           const mtx::client::crypto::IdentityKeys &identity_keys,
                           const mtx::client::crypto::OneTimeKeys &one_time_keys,
                           const mtx::identifiers::User &user_id,
                           const std::string &device_id);

std::string
encode_base64(const uint8_t *data, std::size_t len);

//! Decode the given base64 string
std::unique_ptr<BinaryBuf>
decode_base64(const std::string &data);

//! Convert the given string to an uint8_t buffer.
std::unique_ptr<BinaryBuf>
str_to_buffer(const std::string &data);

//! Convert the given json struct to an uint8_t buffer.
std::unique_ptr<BinaryBuf>
json_to_buffer(const nlohmann::json &obj);

//! Convert from base64 encoded public key.
_olm_curve25519_public_key
str_to_curve25519_pk(const std::string &data);

//! Create an outbount megolm session.
olm::Session
init_outbound_group_session(std::shared_ptr<olm::Account> account,
                            const std::string &peer_id_key,
                            const std::string &peer_one_time_key);

} // namespace crypto
} // namespace client
} // namespace mtx
