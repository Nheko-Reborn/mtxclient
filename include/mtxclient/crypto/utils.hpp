#pragma once

#include <algorithm>
#include <string>
#include <vector>

#include <openssl/aes.h>
#include <openssl/evp.h>
#include <openssl/hmac.h>
#include <openssl/sha.h>

#include <boost/algorithm/string.hpp>

namespace mtx {
namespace crypto {

//! Data representation used to interact with libolm.
using BinaryBuf = std::vector<uint8_t>;

const std::string HEADER_LINE("-----BEGIN MEGOLM SESSION DATA-----");
const std::string TRAILER_LINE("-----END MEGOLM SESSION DATA-----");

//! Simple wrapper around the OpenSSL PKCS5_PBKDF2_HMAC function
BinaryBuf
PBKDF2_HMAC_SHA_512(const std::string pass, const BinaryBuf salt, uint32_t iterations);

BinaryBuf
AES_CTR_256_Encrypt(const std::string plaintext, const BinaryBuf aes256Key, BinaryBuf iv);

BinaryBuf
AES_CTR_256_Decrypt(const std::string ciphertext, const BinaryBuf aes256Key, BinaryBuf iv);

BinaryBuf
HMAC_SHA256(const BinaryBuf hmacKey, const BinaryBuf data);

//! Translates the data back into the binary buffer, taking care
//! to remove the header and footer elements.
std::string
unpack_key_file(const std::string &data);

template<typename T>
void
remove_substrs(std::basic_string<T> &s, const std::basic_string<T> &p);

void
uint8_to_uint32(uint8_t b[4], uint32_t &u32);

void
uint32_to_uint8(uint8_t b[4], uint32_t u32);

void
print_binary_buf(const BinaryBuf buf);

} // namespace crypto
} // namespace mtx