#include "utils.hpp"

#include <iostream>
#include <sstream>

#include <boost/iostreams/copy.hpp>
#include <boost/iostreams/filter/gzip.hpp>
#include <boost/iostreams/filter/zlib.hpp>
#include <boost/iostreams/filtering_stream.hpp>
#include <boost/random/random_device.hpp>
#include <boost/random/uniform_int_distribution.hpp>

std::string
mtx::client::utils::random_token(uint8_t len, bool with_symbols)
{
        std::string symbols = "!@#$%^&*()";
        std::string alphanumberic("abcdefghijklmnopqrstuvwxyz"
                                  "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
                                  "1234567890");

        const auto chars = with_symbols ? alphanumberic + symbols : alphanumberic;

        boost::random::random_device rng;
        boost::random::uniform_int_distribution<> index_dist(0, chars.size() - 1);

        std::string token;
        token.reserve(len);

        for (uint8_t i = 0; i < len; ++i)
                token.push_back(chars[index_dist(rng)]);

        return token;
}

std::string
mtx::client::utils::query_params(const std::map<std::string, std::string> &params)
{
        if (params.empty())
                return "";

        auto pb = params.cbegin();
        auto pe = params.cend();

        std::string data = pb->first + "=" + pb->second;
        ++pb;

        if (pb == pe)
                return data;

        for (; pb != pe; ++pb)
                data += "&" + pb->first + "=" + pb->second;

        return data;
}

std::string
mtx::client::utils::decompress(const boost::iostreams::array_source &src, const std::string &type)
{
        boost::iostreams::filtering_istream is;
        is.set_auto_close(true);

        std::stringstream decompressed;

        if (type == "deflate")
                is.push(boost::iostreams::zlib_decompressor{});
        else if (type == "gzip")
                is.push(boost::iostreams::gzip_decompressor{});

        is.push(src);
        boost::iostreams::copy(is, decompressed);

        return decompressed.str();
}
