#include "mtxclient/crypto/utils.hpp"

#include <iostream>

namespace mtx {
namespace crypto {

BinaryBuf
PBKDF2_HMAC_SHA_512(const std::string pass, const BinaryBuf salt, uint32_t iterations) {
    uint8_t out[SHA512_DIGEST_LENGTH];
    PKCS5_PBKDF2_HMAC(&pass[0], pass.size(), salt.data(), salt.size(), iterations, EVP_sha512(), SHA512_DIGEST_LENGTH, out);
    
    BinaryBuf output(out, out + SHA512_DIGEST_LENGTH);

    return output;
}

BinaryBuf
AES_CTR_256(const std::string plaintext, const BinaryBuf aes256Key, BinaryBuf iv) {
    EVP_CIPHER_CTX *ctx;
    
    int len;

    int ciphertext_len;

    unsigned char *cipher;

    uint8_t *iv_data = iv.data();
    // need to set bit 63 to 0
    *(iv_data) &= ~(1UL << 63);

    /* Create and initialise the context */
    if(!(ctx = EVP_CIPHER_CTX_new())) {
        //handleErrors();
    }

    if(1 != EVP_EncryptInit_ex(ctx, EVP_aes_256_ctr(), NULL, aes256Key.data(), iv_data)) {
        //handleErrors();
    }

    /* Provide the message to be encrypted, and obtain the encrypted output.
    * EVP_EncryptUpdate can be called multiple times if necessary
    */
    if(1 != EVP_EncryptUpdate(ctx, cipher, &len, reinterpret_cast<const unsigned char*>(&plaintext.c_str()[0]), plaintext.size())) {
        //handleErrors();
    }
    ciphertext_len = len;

    /* Finalise the encryption. Further ciphertext bytes may be written at
    * this stage.
    */
    if(1 != EVP_EncryptFinal_ex(ctx, cipher + len, &len)) {
        //handleErrors();
    }

    ciphertext_len += len;

    /* Clean up */
    EVP_CIPHER_CTX_free(ctx);

    return BinaryBuf(reinterpret_cast<uint8_t *>(cipher), cipher + ciphertext_len);

}

BinaryBuf
HMAC_SHA256 (const BinaryBuf hmacKey, const BinaryBuf data) {
    unsigned int len = SHA256_DIGEST_LENGTH;
    unsigned char digest[SHA256_DIGEST_LENGTH];
    HMAC(EVP_sha256(), hmacKey.data(), hmacKey.size(), data.data(), data.size(), digest, &len);
    BinaryBuf output(digest, digest + SHA256_DIGEST_LENGTH);
    return output;
}

void
print_binary_buf(const BinaryBuf buf) {
    for (uint8_t val : buf) {
        std::cout << std::to_string(val) << ",";
    }
    std::cout << std::endl;
}

void uint32_to_uint8 (uint8_t b[4], uint32_t u32) {
    b[3] = (uint8_t)u32;
    b[2] = (uint8_t)(u32>>=8);
    b[1] = (uint8_t)(u32>>=8);
    b[0] = (uint8_t)(u32>>=8);
}

} // namespace crypto
} // namespace mtx
