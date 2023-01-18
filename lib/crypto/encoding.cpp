#include <algorithm>
#include <array>
#include <cassert>
#include <cstdint>
#include <string>
#include <vector>

namespace {
template<std::size_t N, std::size_t... Is>
constexpr std::array<char, N - 1>
to_array(const char (&a)[N], std::index_sequence<Is...>)
{
    return {{a[Is]...}};
}

template<std::size_t N>
constexpr std::array<char, N - 1>
to_array(const char (&a)[N])
{
    return to_array(a, std::make_index_sequence<N - 1>());
}

static constexpr const std::array base64_alphabet =
  to_array("ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/");
static constexpr const std::array base64_urlsafe_alphabet =
  to_array("ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789-_");
static constexpr const std::array base58_alphabet =
  to_array("123456789ABCDEFGHJKLMNPQRSTUVWXYZabcdefghijkmnopqrstuvwxyz");

static_assert(base64_alphabet.size() == 64);
static_assert(base58_alphabet.size() == 58);

template<std::size_t N>
constexpr std::array<uint8_t, 256>
invert_alphabet(std::array<char, N> alphabet)
{
    std::array<uint8_t, 256> inverted{};

    for (auto &e : inverted)
        e = 0xff;

    for (std::size_t i = 0; i < N; i++) {
        inverted[static_cast<uint8_t>(alphabet[i])] = static_cast<uint8_t>(i);
    }

    return inverted;
}

static constexpr const std::array base64_to_int         = invert_alphabet(base64_alphabet);
static constexpr const std::array base64_urlsafe_to_int = invert_alphabet(base64_urlsafe_alphabet);
static constexpr const std::array base58_to_int         = invert_alphabet(base58_alphabet);

static_assert(base64_to_int['A'] == 0);
static_assert(base64_to_int['B'] == 1);
static_assert(base64_to_int['C'] == 2);
static_assert(base64_to_int[0] == 0xff);

// algorithm from https://github.com/miguelmota/cpp-base58 MIT Licensed
inline std::string
encode_base58(const std::array<char, 58> &alphabet, const std::string &input)
{
    if (input.empty())
        return "";

    std::vector<uint8_t> digits(input.size() * 138 / 100 + 1);
    std::size_t digitslen = 1;
    for (uint8_t carry_ : input) {
        uint32_t carry = static_cast<uint32_t>(carry_);
        for (size_t j = 0; j < digitslen; j++) {
            carry += (uint32_t)(digits[j]) * 256;
            digits[j] = static_cast<uint8_t>(carry % 58);
            carry /= 58;
        }
        while (carry > 0) {
            assert(digitslen < digits.size());
            digits[digitslen++] = static_cast<uint8_t>(carry % 58);
            carry /= 58;
        }
    }
    std::string result(digits.size(), ' ');

    // leading zero bytes
    std::size_t resultlen = 0;
    for (; resultlen < input.length() && input[resultlen] == 0;)
        result[resultlen++] = '1';

    // reverse
    for (size_t i = 0; i < digitslen; i++)
        result[resultlen + i] = alphabet[digits[digitslen - 1 - i]];
    result.resize(digitslen + resultlen);

    return result;
}

inline std::string
decode_base58(const std::array<uint8_t, 256> &reverse_alphabet, const std::string &input)
{
    std::string result;
    if (input.empty())
        return result;

    result.reserve(input.size() * 733 / 1000 + 1);

    // result.push_back(0);

    for (uint8_t b : input) {
        if (b == ' ')
            continue;

        if (b == 0xff)
            return "";

        uint32_t carry = reverse_alphabet[b];
        for (char &j : result) {
            carry += static_cast<uint8_t>(j) * 58;
            j = static_cast<char>(static_cast<uint8_t>(carry % 0x100));
            carry /= 0x100;
        }
        while (carry > 0) {
            result.push_back(static_cast<char>(static_cast<uint8_t>(carry % 0x100)));
            carry /= 0x100;
        }
    }

    for (size_t i = 0; i < input.length() && input[i] == '1'; i++)
        result.push_back(0);

    std::reverse(result.begin(), result.end());
    return result;
}

template<bool pad>
inline std::string
encode_base64(const std::array<char, 64> &alphabet, std::string input)
{
    std::string encoded;

    size_t missing = 0;

    while ((input.size() + missing) % 3)
        missing++;

    encoded.reserve((input.size() * 4 + 2) / 3);

    for (size_t i = 0; i < input.size(); i += 3) {
        uint32_t bytes = static_cast<uint8_t>(input[i]) << 16;
        if (i + 1 < input.size())
            bytes += static_cast<uint8_t>(input[i + 1]) << 8;
        if (i + 2 < input.size())
            bytes += static_cast<uint8_t>(input[i + 2]);
        encoded.push_back(alphabet[(bytes >> 18) & 0b11'1111]);
        encoded.push_back(alphabet[(bytes >> 12) & 0b11'1111]);
        encoded.push_back(alphabet[(bytes >> 6) & 0b11'1111]);
        encoded.push_back(alphabet[bytes & 0b11'1111]);
    }

    if constexpr (pad) {
        while (missing) {
            encoded[encoded.size() - missing] = '=';
            missing--;
        }
    }

    encoded.resize(encoded.size() - missing);
    return encoded;
}

inline std::string
decode_base64(const std::array<uint8_t, 256> &reverse_alphabet, const std::string &input)
{
    std::string decoded;
    decoded.reserve((input.size() * 3 + 2) / 4);

    int bit_index = 0;
    uint8_t d     = 0;
    for (uint8_t b : input) {
        if (b == '=')
            break;

        d = reverse_alphabet[b];

        if (d > 64)
            break;

        switch (bit_index++) {
        case 0:
            decoded.push_back(static_cast<char>(d << 2));
            break;
        case 1:
            decoded.back() = static_cast<char>(decoded.back() + (d >> 4));
            decoded.push_back(static_cast<char>(d << 4));
            break;
        case 2:
            decoded.back() = static_cast<char>(decoded.back() + (d >> 2));
            decoded.push_back(static_cast<char>(d << 6));
            break;
        case 3:
            decoded.back() = static_cast<char>(decoded.back() + d);
            bit_index      = 0;
        }
    }

    if ((bit_index == 2 && static_cast<uint8_t>(d << 4) == 0) ||
        (bit_index == 3 && static_cast<uint8_t>(d << 6) == 0))
        decoded.pop_back();

    return decoded;
}
}

namespace mtx {
namespace crypto {
std::string
base642bin(const std::string &b64)
{
    return decode_base64(base64_to_int, b64);
}

std::string
bin2base64(const std::string &bin)
{
    return encode_base64<true>(base64_alphabet, bin);
}

std::string
base642bin_unpadded(const std::string &b64)
{
    return decode_base64(base64_to_int, b64);
}

std::string
bin2base64_unpadded(const std::string &bin)
{
    return encode_base64<false>(base64_alphabet, bin);
}

std::string
base642bin_urlsafe_unpadded(const std::string &b64)
{
    return decode_base64(base64_urlsafe_to_int, b64);
}

std::string
bin2base64_urlsafe_unpadded(const std::string &bin)
{
    return encode_base64<false>(base64_urlsafe_alphabet, bin);
}

std::string
bin2base58(const std::string &bin)
{
    return encode_base58(base58_alphabet, bin);
}

std::string
base582bin(const std::string &bin)
{
    return decode_base58(base58_to_int, bin);
}
}
}
