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
AES_CTR_256_Encrypt(const std::string plaintext, const BinaryBuf aes256Key, BinaryBuf iv) {
    EVP_CIPHER_CTX *ctx;
    
    int len;

    int ciphertext_len;

    unsigned char *cipher = new unsigned char[plaintext.size()];

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

    BinaryBuf encrypted(reinterpret_cast<uint8_t *>(cipher), cipher + ciphertext_len);
    delete [] cipher;
    return encrypted;

}

BinaryBuf
AES_CTR_256_Decrypt(const std::string ciphertext, const BinaryBuf aes256Key, BinaryBuf iv)
{
    EVP_CIPHER_CTX *ctx;

    int len;

    int plaintext_len;

    unsigned char *plaintext = new unsigned char[ciphertext.size()];


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
    if (1 != EVP_DecryptUpdate(ctx, plaintext, &len, reinterpret_cast<const unsigned char*>(&ciphertext.data()[0]), ciphertext.size())) {
            // handleErrors();
    }
    plaintext_len = len;

    /* Finalise the decryption. Further plaintext bytes may be written at
        * this stage.
        */
    if (1 != EVP_DecryptFinal_ex(ctx, plaintext + len, &len)) {
            //  handleErrors();
    }
    plaintext_len += len;

    /* Clean up */
    EVP_CIPHER_CTX_free(ctx);

    BinaryBuf decrypted(reinterpret_cast<uint8_t *>(plaintext), plaintext + plaintext_len);
    delete[] plaintext;
    return decrypted;
}

template<typename T>
void remove_substrs(std::basic_string<T>& s,
                   const std::basic_string<T>& p) {
   auto n = p.length();

   for (auto i = s.find(p);
        i != std::basic_string<T>::npos;
        i = s.find(p))
      s.erase(i, n);
}

std::string
unpack_key_file(const std::string &data) {
    std::string unpacked(data);
    remove_substrs(unpacked, HEADER_LINE);

    remove_substrs(unpacked, TRAILER_LINE);

    remove_substrs(unpacked, std::string("\n"));

    return unpacked;
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

void uint8_to_uint32(uint8_t b[4], uint32_t &u32) {
    u32 = b[0] << 24 | b[1] << 16 | b[2] << 8 | b[3];
}

void uint32_to_uint8 (uint8_t b[4], uint32_t u32) {
    b[3] = (uint8_t)u32;
    b[2] = (uint8_t)(u32>>=8);
    b[1] = (uint8_t)(u32>>=8);
    b[0] = (uint8_t)(u32>>=8);
}

} // namespace crypto
} // namespace mtx
