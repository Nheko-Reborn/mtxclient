#pragma once

#include <boost/iostreams/device/array.hpp>
#include <iosfwd>
#include <map>

#include <nlohmann/json.hpp>

namespace mtx {
namespace client {
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

//! Decompress a response.
std::string
decompress(const boost::iostreams::array_source &src, const std::string &type) noexcept;

//! URL-encode the input string.
std::string
url_encode(const std::string &s) noexcept;

template<class T>
inline T
deserialize(const std::string &data)
{
        return nlohmann::json::parse(data);
}

template<>
inline std::string
deserialize<std::string>(const std::string &data)
{
        return data;
}

template<class T>
inline std::string
serialize(const T &obj)
{
        return nlohmann::json(obj).dump();
}

template<>
inline std::string
serialize<std::string>(const std::string &obj)
{
        return obj;
}

template<class T, class Name>
class strong_type
{
public:
        strong_type() = default;
        explicit strong_type(const T &value)
          : value_(value)
        {}
        explicit strong_type(T &&value)
          : value_(std::forward<T>(value))
        {}

        operator T &() noexcept { return value_; }
        constexpr operator const T &() const noexcept { return value_; }

        T &get() { return value_; }
        T const &get() const { return value_; }

private:
        T value_;
};

// Macro for concisely defining a strong type
#define STRONG_TYPE(type_name, value_type)                                                         \
        struct type_name : mtx::client::utils::strong_type<value_type, type_name>                  \
        {                                                                                          \
                using strong_type::strong_type;                                                    \
        };
}
}
}
