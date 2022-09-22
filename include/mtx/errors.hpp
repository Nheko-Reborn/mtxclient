#pragma once

/// @file
/// @brief The error struct returned by the Matrix API.

#include <chrono>

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

    //! Retry delay in case of 429
    std::chrono::duration<std::uint64_t, std::milli> retry_after;

    friend void from_json(const nlohmann::json &obj, Error &error);
};
}
}
