#pragma once

#include <exception>
#include <memory>

#include <json.hpp>
#include <mtx/identifiers.hpp>
#include <olm/account.hh>
#include <olm/error.h>

namespace mtx {
namespace client {
namespace crypto {

class olm_exception : public std::exception
{
public:
        olm_exception(std::string msg, OlmErrorCode errcode)
          : errcode_(errcode)
          , msg_(msg + ": " + std::string(_olm_error_to_string(errcode)))
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
nlohmann::json
identity_keys(std::shared_ptr<olm::Account> user);

//! Generate a number of one time keys.
std::size_t
generate_one_time_keys(std::shared_ptr<olm::Account> account, std::size_t number_of_keys);

//! Retrieve the json representation of the one time keys for the given account.
nlohmann::json
one_time_keys(std::shared_ptr<olm::Account> user);

//! Create a uint8_t buffer which is initialized with random bytes.
std::unique_ptr<uint8_t[]>
create_buffer(std::size_t nbytes);

//! Sign the given one time keys and encode it to base64.
std::string
sign_one_time_key(std::shared_ptr<olm::Account> account, const std::string &key);

//! Sign the given message.
std::unique_ptr<uint8_t[]>
sign_message(std::shared_ptr<olm::Account> account, const std::string &msg);

//! Generate the json structure for the signed one time key.
nlohmann::json
signed_one_time_key_json(const mtx::identifiers::User &user_id,
                         const std::string &device_id,
                         const std::string &key,
                         const std::string &signature);

std::string
encode_base64(const uint8_t *data, std::size_t len);

//! Decode the given base64 string
std::unique_ptr<uint8_t[]>
decode_base64(const std::string &data);

//! Convert the given string to an uint8_t buffer.
std::unique_ptr<uint8_t[]>
str_to_buffer(const std::string &data);

//! Convert the given json struct to an uint8_t buffer.
std::unique_ptr<uint8_t[]>
json_to_buffer(const nlohmann::json &obj);

} // namespace crypto
} // namespace client
} // namespace mtx
