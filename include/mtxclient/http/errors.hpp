#pragma once

/// @file
/// @brief Error codes returned by the client-server API

#include "mtx/errors.hpp"

namespace mtx {
namespace http {

//! Compound type that includes matrix & network related errors.
struct ClientError
{
    //! Matrix client api related error.
    mtx::errors::Error matrix_error;
    //! Error code if a network related error occured.
    int error_code;
    //! Status code of the associated http response.
    int status_code;
    //! Parsing response error.
    std::string parse_error;

    const char *error_code_string() const;
};
} // namespace http
} // namespace mtx
