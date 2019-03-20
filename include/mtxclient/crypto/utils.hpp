#pragma once


#include <string>
#include <vector>

#include <openssl/aes.h>
#include <openssl/evp.h>
#include <openssl/hmac.h>
#include <openssl/sha.h>


namespace mtx {
namespace crypto {

//! Data representation used to interact with libolm.
using BinaryBuf = std::vector<uint8_t>;

//! Simple wrapper around the OpenSSL PKCS5_PBKDF2_HMAC function
BinaryBuf PBKDF2_HMAC_SHA_512(const std::string pass, const BinaryBuf salt, uint32_t iterations);

BinaryBuf AES_CTR_256(const std::string plaintext, const BinaryBuf aes256Key, BinaryBuf iv);

BinaryBuf HMAC_SHA256(const BinaryBuf hmacKey, const BinaryBuf data);

void uint32_to_uint8 (uint8_t b[4], uint32_t u32);

void print_binary_buf(const BinaryBuf buf) ;

} // namespace crypto
} // namespace mtx