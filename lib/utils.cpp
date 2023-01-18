#include "mtxclient/utils.hpp"

#include <algorithm>
#include <cstdint>
#include <iomanip>
#include <random>
#include <sstream>
#include <string>
#include <utility>

mtx::client::utils::MxcUrl
mtx::client::utils::parse_mxc_url(const std::string &url)
{
    constexpr auto mxc_uri_protocol = "mxc://";
    MxcUrl res;

    if (url.find(mxc_uri_protocol) != 0)
        return res;

    auto str_left = url.substr(6);

    std::vector<std::string> parts;

    size_t pos = 0;
    while ((pos = str_left.find('/')) != std::string_view::npos) {
        parts.push_back(std::string(str_left.substr(0, pos)));
        str_left = str_left.substr(pos + 1);
    }
    parts.emplace_back(str_left);

    if (parts.size() != 2) {
        res.server = parts.at(0);
        return res;
    }

    res.server   = parts.at(0);
    res.media_id = parts.at(1);

    return res;
}

bool
mtx::client::utils::is_number(const std::string &s)
{
    return !s.empty() &&
           std::find_if(s.begin(), s.end(), [](char c) { return !std::isdigit(c); }) == s.end();
}

std::string
mtx::client::utils::random_token(uint8_t len, bool with_symbols) noexcept
{
    std::string symbols = "!@#$%^&*()";
    std::string alphanumberic("abcdefghijklmnopqrstuvwxyz"
                              "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
                              "1234567890");

    const auto chars = with_symbols ? alphanumberic + symbols : alphanumberic;

    thread_local std::random_device rng;
    std::uniform_int_distribution<> index_dist(0, (int)chars.size() - 1);

    std::string token;
    token.reserve(len);

    for (uint8_t i = 0; i < len; ++i)
        token.push_back(chars[index_dist(rng)]);

    return token;
}

std::string
mtx::client::utils::query_params(const std::map<std::string, std::string> &params) noexcept
{
    if (params.empty())
        return "";

    auto pb = params.cbegin();
    auto pe = params.cend();

    std::string data = pb->first + "=" + url_encode(pb->second);
    ++pb;

    if (pb == pe)
        return data;

    for (; pb != pe; ++pb)
        data += "&" + pb->first + "=" + url_encode(pb->second);

    return data;
}

std::string
mtx::client::utils::url_encode(const std::string &value) noexcept
{
    // https: // stackoverflow.com/questions/154536/encode-decode-urls-in-c
    std::ostringstream escaped;
    escaped.fill('0');
    escaped << std::hex;

    for (char c : value) {
        // Keep alphanumeric and other accepted characters intact
        if (std::isalnum(c) || c == '-' || c == '_' || c == '.' || c == '~') {
            escaped << c;
            continue;
        }

        // Any other characters are percent-encoded
        escaped << std::uppercase;
        escaped << '%' << std::setw(2) << int((unsigned char)c);
        escaped << std::nouppercase;
    }

    return escaped.str();
}
