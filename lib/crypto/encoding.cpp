#include <algorithm>
#include <array>

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
constexpr std::array<int, 256>
invert_alphabet(std::array<char, N> alphabet)
{
        std::array<int, 256> inverted{};

        for (auto &e : inverted)
                e = -1;

        for (std::size_t i = 0; i < N; i++) {
                inverted[static_cast<unsigned char>(alphabet[i])] = i;
        }

        return inverted;
}

static constexpr const std::array base64_to_int         = invert_alphabet(base64_alphabet);
static constexpr const std::array base64_urlsafe_to_int = invert_alphabet(base64_urlsafe_alphabet);
static constexpr const std::array base58_to_int         = invert_alphabet(base58_alphabet);

// algorithm from https://github.com/miguelmota/cpp-base58 MIT Licensed
inline std::string
encode_base58(std::array<char, 58> alphabet, const std::string &input)
{
        if (input.empty())
                return "";

        std::vector<uint8_t> digits(input.size() * 137 / 100 + 1);
        std::size_t digitslen = 1;
        for (uint32_t carry : input) {
                for (size_t j = 0; j < digitslen; j++) {
                        carry += (uint32_t)(digits[j]) << 8;
                        digits[j] = static_cast<uint8_t>(carry % 58);
                        carry /= 58;
                }
                while (carry > 0) {
                        digits[digitslen++] = static_cast<uint8_t>(carry % 58);
                        carry /= 58;
                }
        }
        std::size_t resultlen = 0;
        std::string result(digits.size(), ' ');

        // leading zero bytes
        for (; resultlen < input.length() && input[resultlen] == 0;)
                result[resultlen++] = '1';

        // reverse
        for (size_t i = 0; i < digitslen; i++)
                result[resultlen + i] = alphabet[digits[digitslen - 1 - i]];
        result.resize(digitslen + resultlen);

        return result;
}

template<bool pad>
inline std::string
encode_base64(std::array<char, 64> alphabet, std::string input)
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

namespace mtx {
namespace crypto {
std::string
base642bin(const std::string &b64);

std::string
bin2base64(const std::string &bin)
{
        return encode_base64<true>(base64_alphabet, bin);
}

std::string
base642bin_unpadded(const std::string &b64);

std::string
bin2base64_unpadded(const std::string &bin)
{
        return encode_base64<false>(base64_alphabet, bin);
}

std::string
base642bin_urlsafe_unpadded(const std::string &b64);

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
}
}
