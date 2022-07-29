#pragma once

/// @file
/// @brief The error struct returned by the Matrix API.

#include "lightweight_error.hpp"
#include "user_interactive.hpp"

namespace mtx {
namespace errors {
//! Represents a Matrix related error.
struct Error
{
    //! Error code.
    ErrorCode errcode = {};
    //! Human readable version of the error.
    std::string error;

    //! Auth flows in case of 401
    user_interactive::Unauthorized unauthorized;

    friend void from_json(const nlohmann::json &obj, Error &error);
};
}
}
