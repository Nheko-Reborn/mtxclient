#pragma once

/// @file
/// @brief Various crypto functions.

#include <string>
#include <vector>

#include "mtx/common.hpp"
#include "mtx/responses/crypto.hpp"
#include "mtx/secret_storage.hpp"

namespace mtx {
namespace crypto {
//! Exception thrown for various encryption related errors (not reported by olm), that generally
//! should not be ignored.
class crypto_exception : public std::exception
{
public:
    crypto_exception(std::string func, const char *msg)
      : msg_(func + ": " + std::string(msg))
    {}

    //! Describes the error
    const char *what() const noexcept override { return msg_.c_str(); }

private:
    std::string msg_;
};

//! Data representation used to interact with libolm. It's a contiguous buffer of bytes.
using BinaryBuf = std::vector<uint8_t>;

const std::string HEADER_LINE("-----BEGIN MEGOLM SESSION DATA-----");
const std::string TRAILER_LINE("-----END MEGOLM SESSION DATA-----");

//! Create a uint8_t buffer which is initialized with random bytes.
BinaryBuf
create_buffer(std::size_t nbytes);

//! Convert a string to a binary buffer.
inline BinaryBuf
to_binary_buf(const std::string &str)
{
    return BinaryBuf(reinterpret_cast<const uint8_t *>(str.data()),
                     reinterpret_cast<const uint8_t *>(str.data()) + str.size());
}

//! Convert a binary buffer to a string.
inline std::string
to_string(const BinaryBuf &buf)
{
    return std::string(reinterpret_cast<const char *>(buf.data()), buf.size());
}

//! Sets bit 63 to 0 to be compatible with other AES implementations.
BinaryBuf
compatible_iv(BinaryBuf incompatible_iv);

//! encodes a recovery key in base58 with parity and version tag,
std::string
key_to_recoverykey(const BinaryBuf &key);

//! Simple wrapper around the OpenSSL PKCS5_PBKDF2_HMAC function
BinaryBuf
PBKDF2_HMAC_SHA_512(const std::string &pass,
                    const BinaryBuf &salt,
                    uint32_t iterations,
                    uint32_t keylen = 64);

//! Derive the SSSS decryption key from a passphrase using the parameters stored in account_data.
std::optional<BinaryBuf>
key_from_passphrase(const std::string &password,
                    const mtx::secret_storage::AesHmacSha2KeyDescription &parameters);

//! Derive the SSSS decryption key from a base58 encoded recoverykey using the parameters stored in
//! account_data.
std::optional<BinaryBuf>
key_from_recoverykey(const std::string &recoverkey,
                     const mtx::secret_storage::AesHmacSha2KeyDescription &parameters);

//! Decrypt a secret from SSSS
std::string
decrypt(const mtx::secret_storage::AesHmacSha2EncryptedData &data,
        const BinaryBuf &decryptionKey,
        const std::string &key_name);

//! Encrypt a secret for SSSS
mtx::secret_storage::AesHmacSha2EncryptedData
encrypt(const std::string &data, const BinaryBuf &decryptionKey, const std::string &key_name);

//! HKDF key derivation with SHA256 digest
struct HkdfKeys
{
    BinaryBuf aes, mac;
};
HkdfKeys
HKDF_SHA256(const BinaryBuf &key, const BinaryBuf &salt, const BinaryBuf &info);

BinaryBuf
AES_CTR_256_Encrypt(const std::string &plaintext, const BinaryBuf &aes256Key, BinaryBuf iv);

BinaryBuf
AES_CTR_256_Decrypt(const std::string &ciphertext, const BinaryBuf &aes256Key, BinaryBuf iv);

//! Base64 encoded CURVE25519_AES_SHA2 encrypted text, including the mac and ephemeral key
struct CURVE25519_AES_SHA2_Encrypted
{
    //! base64 encoded
    std::string ciphertext, mac, ephemeral;
};

//! encypts a plaintext payload using CURVE25519_AES_SHA2
CURVE25519_AES_SHA2_Encrypted
CURVE25519_AES_SHA2_Encrypt(const std::string &plaintext, const std::string &base64_publicKey);

//! returns base64 encoded pubkey
std::string
CURVE25519_public_key_from_private(const BinaryBuf &privateKey);

// copies ciphertext, as decryption modifies it.
std::string
CURVE25519_AES_SHA2_Decrypt(std::string base64_ciphertext,
                            const BinaryBuf &privateKey,
                            const std::string &ephemeral,
                            const std::string &mac);

//! encrypt a session for online key backup
mtx::responses::backup::EncryptedSessionData
encrypt_session(const mtx::responses::backup::SessionData &data, const std::string &publicKey);

//! Decrypt a session retrieved from online key backup.
mtx::responses::backup::SessionData
decrypt_session(const mtx::responses::backup::EncryptedSessionData &data,
                const BinaryBuf &privateKey);

BinaryBuf
HMAC_SHA256(const BinaryBuf &hmacKey, const BinaryBuf &data);

//! Sha256 a string.
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

//! Convert base64 to binary
std::string
base642bin(const std::string &b64);

//! Encode a binary string in base64.
std::string
bin2base64(const std::string &bin);

//! Decode unpadded base64 to binary.
std::string
base642bin_unpadded(const std::string &b64);

//! Encode binary in unpadded base64.
std::string
bin2base64_unpadded(const std::string &bin);

//! Decode urlsafe, unpadded base64 to binary.
std::string
base642bin_urlsafe_unpadded(const std::string &b64);

//! Encode binary in urlsafe, unpadded base64.
std::string
bin2base64_urlsafe_unpadded(const std::string &bin);

//! Encode binary in base58.
std::string
bin2base58(const std::string &bin);

//! Decode base58 to binary.
std::string
base582bin(const std::string &bin);
} // namespace crypto
} // namespace mtx
