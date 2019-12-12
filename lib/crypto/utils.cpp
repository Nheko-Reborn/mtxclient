#include "mtxclient/crypto/utils.hpp"

#include <openssl/aes.h>
#include <openssl/evp.h>
#include <openssl/hmac.h>
#include <openssl/sha.h>

#include <algorithm>
#include <iomanip>
#include <iostream>

namespace mtx {
namespace crypto {

BinaryBuf
PBKDF2_HMAC_SHA_512(const std::string pass, const BinaryBuf salt, uint32_t iterations)
{
        uint8_t out[SHA512_DIGEST_LENGTH];
        PKCS5_PBKDF2_HMAC(&pass[0],
                          pass.size(),
                          salt.data(),
                          salt.size(),
                          iterations,
                          EVP_sha512(),
                          SHA512_DIGEST_LENGTH,
                          out);

        BinaryBuf output(out, out + SHA512_DIGEST_LENGTH);

        return output;
}

BinaryBuf
AES_CTR_256_Encrypt(const std::string plaintext, const BinaryBuf aes256Key, BinaryBuf iv)
{
        EVP_CIPHER_CTX *ctx;

        int len;

        int ciphertext_len;

        // The ciphertext expand up to block size, which is 128 for AES256
        BinaryBuf encrypted = create_buffer(plaintext.size() + AES_BLOCK_SIZE);

        uint8_t *iv_data = iv.data();
        // need to set bit 63 to 0
        *(iv_data) &= ~(1UL << 63);

        /* Create and initialise the context */
        if (!(ctx = EVP_CIPHER_CTX_new())) {
                // handleErrors();
        }

        if (1 != EVP_EncryptInit_ex(ctx, EVP_aes_256_ctr(), NULL, aes256Key.data(), iv_data)) {
                // handleErrors();
        }

        /* Provide the message to be encrypted, and obtain the encrypted output.
         * EVP_EncryptUpdate can be called multiple times if necessary
         */
        if (1 != EVP_EncryptUpdate(ctx,
                                   encrypted.data(),
                                   &len,
                                   reinterpret_cast<const unsigned char *>(&plaintext.c_str()[0]),
                                   plaintext.size())) {
                // handleErrors();
        }
        ciphertext_len = len;

        /* Finalise the encryption. Further ciphertext bytes may be written at
         * this stage.
         */
        if (1 != EVP_EncryptFinal_ex(ctx, encrypted.data() + len, &len)) {
                // handleErrors();
        }

        ciphertext_len += len;
        encrypted.resize(ciphertext_len);

        /* Clean up */
        EVP_CIPHER_CTX_free(ctx);

        return encrypted;
}

BinaryBuf
AES_CTR_256_Decrypt(const std::string ciphertext, const BinaryBuf aes256Key, BinaryBuf iv)
{
        EVP_CIPHER_CTX *ctx;

        int len;

        int plaintext_len;

        BinaryBuf decrypted = create_buffer(ciphertext.size());

        /* Create and initialise the context */
        if (!(ctx = EVP_CIPHER_CTX_new())) {
                // handleErrors();
        }

        /* Initialise the decryption operation. IMPORTANT - ensure you use a key
         * and IV size appropriate for your cipher
         * In this example we are using 256 bit AES (i.e. a 256 bit key). The
         * IV size for *most* modes is the same as the block size. For AES this
         * is 128 bits */
        if (1 != EVP_DecryptInit_ex(ctx, EVP_aes_256_ctr(), NULL, aes256Key.data(), iv.data())) {
                // handleErrors();
        }

        /* Provide the message to be decrypted, and obtain the plaintext output.
         * EVP_DecryptUpdate can be called multiple times if necessary
         */
        if (1 != EVP_DecryptUpdate(ctx,
                                   decrypted.data(),
                                   &len,
                                   reinterpret_cast<const unsigned char *>(&ciphertext.data()[0]),
                                   ciphertext.size())) {
                // handleErrors();
        }
        plaintext_len = len;

        /* Finalise the decryption. Further plaintext bytes may be written at
         * this stage.
         */
        if (1 != EVP_DecryptFinal_ex(ctx, decrypted.data() + len, &len)) {
                //  handleErrors();
        }
        plaintext_len += len;
        decrypted.resize(plaintext_len);

        /* Clean up */
        EVP_CIPHER_CTX_free(ctx);

        return decrypted;
}

std::string
sha256(const std::string &data)
{
        bool success = false;
        std::string hashed;

#if OPENSSL_VERSION_NUMBER < 0x10100000L
        EVP_MD_CTX *context = EVP_MD_CTX_create();
#else
        EVP_MD_CTX *context = EVP_MD_CTX_new();
#endif

        if (context != NULL) {
                if (EVP_DigestInit_ex(context, EVP_sha256(), NULL)) {
                        if (EVP_DigestUpdate(context, data.c_str(), data.length())) {
                                unsigned char hash[EVP_MAX_MD_SIZE];
                                unsigned int lengthOfHash = 0;

                                if (EVP_DigestFinal_ex(context, hash, &lengthOfHash)) {
                                        hashed  = std::string(hash, hash + lengthOfHash);
                                        success = true;
                                }
                        }
                }

#if OPENSSL_VERSION_NUMBER < 0x10100000L
                EVP_MD_CTX_destroy(context);
#else
                EVP_MD_CTX_free(context);
#endif
        }

        if (success)
                return hashed;
        throw std::runtime_error("sha256 failed!");
}

BinaryBuf
decrypt_file(const std::string &ciphertext, const mtx::crypto::EncryptedFile &encryption_info)
{
        if (encryption_info.v != "v2")
                throw std::invalid_argument("Unsupported encrypted file version");

        if (encryption_info.key.kty != "oct")
                throw std::invalid_argument("Unsupported key type");

        if (encryption_info.key.alg != "A256CTR")
                throw std::invalid_argument("Unsupported algorithm");

        // Be careful, the key should be urlsafe and unpadded, the iv and sha only need to
        // be unpadded
        if (bin2base64_unpadded(sha256(ciphertext)) != encryption_info.hashes.at("sha256"))
                throw std::invalid_argument(
                  "sha256 of encrypted file does not match the ciphertext, expected '" +
                  bin2base64_unpadded(sha256(ciphertext)) + "', got '" +
                  encryption_info.hashes.at("sha256") + "'");

        return AES_CTR_256_Decrypt(
          ciphertext,
          to_binary_buf(base642bin_urlsafe_unpadded(encryption_info.key.k)),
          to_binary_buf(base642bin_unpadded(encryption_info.iv)));
}

std::pair<BinaryBuf, mtx::crypto::EncryptedFile>
encrypt_file(const std::string &plaintext)
{
        mtx::crypto::EncryptedFile encryption_info;

        // iv has to be 16 bytes, key 32!
        BinaryBuf key = create_buffer(32);
        BinaryBuf iv  = create_buffer(16);

        BinaryBuf cyphertext = AES_CTR_256_Encrypt(plaintext, key, iv);

        // Be careful, the key should be urlsafe and unpadded, the iv and sha only need to
        // be unpadded
        JWK web_key;
        web_key.ext     = true;
        web_key.kty     = "oct";
        web_key.key_ops = {"encrypt", "decrypt"};
        web_key.alg     = "A256CTR";
        web_key.k       = bin2base64_urlsafe_unpadded(to_string(key));
        web_key.ext     = true;

        encryption_info.key              = web_key;
        encryption_info.iv               = bin2base64_unpadded(to_string(iv));
        encryption_info.hashes["sha256"] = bin2base64_unpadded(sha256(to_string(cyphertext)));
        encryption_info.v                = "v2";

        return std::make_pair(cyphertext, encryption_info);
}

template<typename T>
void
remove_substrs(std::basic_string<T> &s, const std::basic_string<T> &p)
{
        auto n = p.length();

        for (auto i = s.find(p); i != std::basic_string<T>::npos; i = s.find(p))
                s.erase(i, n);
}

std::string
unpack_key_file(const std::string &data)
{
        std::string unpacked(data);
        remove_substrs(unpacked, HEADER_LINE);

        remove_substrs(unpacked, TRAILER_LINE);

        remove_substrs(unpacked, std::string("\n"));

        return unpacked;
}

BinaryBuf
HMAC_SHA256(const BinaryBuf hmacKey, const BinaryBuf data)
{
        unsigned int len = SHA256_DIGEST_LENGTH;
        unsigned char digest[SHA256_DIGEST_LENGTH];
        HMAC(EVP_sha256(), hmacKey.data(), hmacKey.size(), data.data(), data.size(), digest, &len);
        BinaryBuf output(digest, digest + SHA256_DIGEST_LENGTH);
        return output;
}

void
print_binary_buf(const BinaryBuf buf)
{
        for (uint8_t val : buf) {
                std::cout << std::to_string(val) << ",";
        }
        std::cout << std::endl;
}

void
uint8_to_uint32(uint8_t b[4], uint32_t &u32)
{
        u32 = b[0] << 24 | b[1] << 16 | b[2] << 8 | b[3];
}

void
uint32_to_uint8(uint8_t b[4], uint32_t u32)
{
        b[3] = (uint8_t)u32;
        b[2] = (uint8_t)(u32 >>= 8);
        b[1] = (uint8_t)(u32 >>= 8);
        b[0] = (uint8_t)(u32 >>= 8);
}

std::string
base642bin(const std::string &b64)
{
        std::size_t bin_maxlen = b64.size();
        std::size_t bin_len;

        const char *max_end;

        auto ciphertext = create_buffer(bin_maxlen);

        const int rc = sodium_base642bin(reinterpret_cast<unsigned char *>(ciphertext.data()),
                                         ciphertext.size(),
                                         b64.data(),
                                         b64.size(),
                                         nullptr,
                                         &bin_len,
                                         &max_end,
                                         sodium_base64_VARIANT_ORIGINAL);
        if (rc != 0)
                throw sodium_exception{"sodium_base642bin", "encoding failed"};

        if (bin_len != bin_maxlen)
                ciphertext.resize(bin_len);

        return std::string(std::make_move_iterator(ciphertext.begin()),
                           std::make_move_iterator(ciphertext.end()));
}

std::string
bin2base64(const std::string &bin)
{
        auto base64buf =
          create_buffer(sodium_base64_encoded_len(bin.size(), sodium_base64_VARIANT_ORIGINAL));

        sodium_bin2base64(reinterpret_cast<char *>(base64buf.data()),
                          base64buf.size(),
                          reinterpret_cast<const unsigned char *>(bin.data()),
                          bin.size(),
                          sodium_base64_VARIANT_ORIGINAL);

        // Removing the null byte.
        return std::string(base64buf.begin(), base64buf.end() - 1);
}
std::string
base642bin_unpadded(const std::string &b64)
{
        std::size_t bin_maxlen = b64.size();
        std::size_t bin_len;

        const char *max_end;

        auto ciphertext = create_buffer(bin_maxlen);

        const int rc = sodium_base642bin(reinterpret_cast<unsigned char *>(ciphertext.data()),
                                         ciphertext.size(),
                                         b64.data(),
                                         b64.size(),
                                         nullptr,
                                         &bin_len,
                                         &max_end,
                                         sodium_base64_VARIANT_ORIGINAL_NO_PADDING);
        if (rc != 0)
                throw sodium_exception{"sodium_base642bin", "encoding failed"};

        if (bin_len != bin_maxlen)
                ciphertext.resize(bin_len);

        return std::string(std::make_move_iterator(ciphertext.begin()),
                           std::make_move_iterator(ciphertext.end()));
}

std::string
bin2base64_unpadded(const std::string &bin)
{
        auto base64buf = create_buffer(
          sodium_base64_encoded_len(bin.size(), sodium_base64_VARIANT_ORIGINAL_NO_PADDING));

        sodium_bin2base64(reinterpret_cast<char *>(base64buf.data()),
                          base64buf.size(),
                          reinterpret_cast<const unsigned char *>(bin.data()),
                          bin.size(),
                          sodium_base64_VARIANT_ORIGINAL_NO_PADDING);

        // Removing the null byte.
        return std::string(base64buf.begin(), base64buf.end() - 1);
}
std::string
base642bin_urlsafe_unpadded(const std::string &b64)
{
        std::size_t bin_maxlen = b64.size();
        std::size_t bin_len;

        const char *max_end;

        auto ciphertext = create_buffer(bin_maxlen);

        const int rc = sodium_base642bin(reinterpret_cast<unsigned char *>(ciphertext.data()),
                                         ciphertext.size(),
                                         b64.data(),
                                         b64.size(),
                                         nullptr,
                                         &bin_len,
                                         &max_end,
                                         sodium_base64_VARIANT_URLSAFE_NO_PADDING);
        if (rc != 0)
                throw sodium_exception{"sodium_base642bin", "encoding failed"};

        if (bin_len != bin_maxlen)
                ciphertext.resize(bin_len);

        return std::string(std::make_move_iterator(ciphertext.begin()),
                           std::make_move_iterator(ciphertext.end()));
}

std::string
bin2base64_urlsafe_unpadded(const std::string &bin)
{
        auto base64buf = create_buffer(
          sodium_base64_encoded_len(bin.size(), sodium_base64_VARIANT_URLSAFE_NO_PADDING));

        sodium_bin2base64(reinterpret_cast<char *>(base64buf.data()),
                          base64buf.size(),
                          reinterpret_cast<const unsigned char *>(bin.data()),
                          bin.size(),
                          sodium_base64_VARIANT_URLSAFE_NO_PADDING);

        // Removing the null byte.
        return std::string(base64buf.begin(), base64buf.end() - 1);
}

} // namespace crypto
} // namespace mtx
