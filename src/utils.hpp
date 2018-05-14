#pragma once

#include <boost/iostreams/device/array.hpp>
#include <iosfwd>
#include <map>

#include <json.hpp>

namespace mtx {
namespace client {
namespace utils {

//! Generates a random string of the given size.
std::string
random_token(uint8_t len = 12, bool with_symbols = true) noexcept;

//! Construct query string from the given parameter pairs.
std::string
query_params(const std::map<std::string, std::string> &params) noexcept;

//! Decompress a response.
std::string
decompress(const boost::iostreams::array_source &src, const std::string &type) noexcept;

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
}
}
}
