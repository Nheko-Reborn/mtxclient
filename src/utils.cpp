#include "utils.hpp"

#include <boost/random/random_device.hpp>
#include <boost/random/uniform_int_distribution.hpp>

std::string
mtx::client::utils::random_token(uint8_t len)
{
        std::string chars("abcdefghijklmnopqrstuvwxyz"
                          "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
                          "1234567890"
                          "!@#$%^&*()");
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
