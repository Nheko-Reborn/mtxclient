#pragma once

/// @file
/// @brief Various utility functions for http requests.

#include <iosfwd>
#include <map>
#include <string>

namespace mtx {
namespace client {
//! Utility namespace.
namespace utils {

//! Representation of Matrix Content (MXC) URIs.
struct MxcUrl
{
    //! The name of the homeserver where the content originated.
    std::string server;
    //! An opaque ID which identifies the content.
    std::string media_id;
};

//! Parse a matrix content URI into its server & media_id components.
MxcUrl
parse_mxc_url(const std::string &url);

//! Check if the given string represents a number.
bool
is_number(const std::string &s);

//! Generates a random string of the given size.
std::string
random_token(uint8_t len = 12, bool with_symbols = true) noexcept;

//! Construct query string from the given parameter pairs.
std::string
query_params(const std::map<std::string, std::string> &params) noexcept;

//! URL-encode the input string.
std::string
url_encode(const std::string &s) noexcept;
}
}
}
