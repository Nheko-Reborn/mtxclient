#pragma once

#include <string>
#include <vector>

#include <sodium.h>

#include "mtx/common.hpp"

namespace mtx {
namespace crypto {
class sodium_exception : public std::exception
{
public:
        sodium_exception(std::string func, const char *msg)
          : msg_(func + ": " + std::string(msg))
        {}

        virtual const char *what() const throw() { return msg_.c_str(); }

private:
        std::string msg_;
};

//! Data representation used to interact with libolm.
using BinaryBuf = std::vector<uint8_t>;

const std::string HEADER_LINE("-----BEGIN MEGOLM SESSION DATA-----");
const std::string TRAILER_LINE("-----END MEGOLM SESSION DATA-----");

//! Create a uint8_t buffer which is initialized with random bytes.
inline BinaryBuf
create_buffer(std::size_t nbytes)
{
        auto buf = BinaryBuf(nbytes);
        randombytes_buf(buf.data(), buf.size());

        return buf;
}

inline BinaryBuf
to_binary_buf(const std::string &str)
{
        return BinaryBuf(reinterpret_cast<const uint8_t *>(str.data()),
                         reinterpret_cast<const uint8_t *>(str.data()) + str.size());
}

inline std::string
to_string(const BinaryBuf &buf)
{
        return std::string(reinterpret_cast<const char *>(buf.data()), buf.size());
}

//! Simple wrapper around the OpenSSL PKCS5_PBKDF2_HMAC function
BinaryBuf
PBKDF2_HMAC_SHA_512(const std::string pass, const BinaryBuf salt, uint32_t iterations);

BinaryBuf
AES_CTR_256_Encrypt(const std::string plaintext, const BinaryBuf aes256Key, BinaryBuf iv);

BinaryBuf
AES_CTR_256_Decrypt(const std::string ciphertext, const BinaryBuf aes256Key, BinaryBuf iv);

BinaryBuf
HMAC_SHA256(const BinaryBuf hmacKey, const BinaryBuf data);

std::string
sha256(const std::string &data);

//! Decrypt matrix EncryptedFile
BinaryBuf
decrypt_file(const std::string &ciphertext, const mtx::crypto::EncryptedFile &encryption_info);

//! Encrypt matrix EncryptedFile
// Remember to set the url member of the EncryptedFile struct!
std::pair<BinaryBuf, mtx::crypto::EncryptedFile>
encrypt_file(const std::string &plaintext);

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

std::string
base642bin(const std::string &b64);

std::string
bin2base64(const std::string &bin);

std::string
base642bin_unpadded(const std::string &b64);

std::string
bin2base64_unpadded(const std::string &bin);

std::string
base642bin_urlsafe_unpadded(const std::string &b64);

std::string
bin2base64_urlsafe_unpadded(const std::string &bin);

} // namespace crypto
} // namespace mtx
