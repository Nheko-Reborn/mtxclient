#pragma once

#include <exception>
#include <memory>

#include <json.hpp>
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

} // namespace crypto
} // namespace client
} // namespace mtx
