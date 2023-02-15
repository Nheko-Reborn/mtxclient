#include "mtxclient/crypto/utils.hpp"

#include <nlohmann/json.hpp>

#include <openssl/aes.h>
#include <openssl/evp.h>
#include <openssl/hmac.h>
#include <openssl/kdf.h>
#include <openssl/rand.h>
#include <openssl/sha.h>

#include <olm/pk.h>

#include <algorithm>
#include <cstdint>

#include "mtx/log.hpp"
#include "mtxclient/crypto/client.hpp"

namespace mtx {
namespace crypto {
BinaryBuf
create_buffer(std::size_t nbytes)
{
    auto buf = BinaryBuf(nbytes);
    RAND_bytes(buf.data(), (int)buf.size());

    return buf;
}

BinaryBuf
PBKDF2_HMAC_SHA_512(const std::string &pass,
                    const BinaryBuf &salt,
                    uint32_t iterations,
                    uint32_t keylen)
{
    BinaryBuf out(keylen);
    PKCS5_PBKDF2_HMAC(&pass[0],
                      (int)pass.size(),
                      salt.data(),
                      (int)salt.size(),
                      (int)iterations,
                      EVP_sha512(),
                      (int)keylen,
                      out.data());

    return out;
}

std::optional<BinaryBuf>
key_from_passphrase(const std::string &password,
                    const mtx::secret_storage::AesHmacSha2KeyDescription &parameters)
{
    if (!parameters.passphrase)
        throw std::invalid_argument("no passphrase to derive key from");
    if (parameters.passphrase->algorithm != "m.pbkdf2")
        throw std::invalid_argument("invalid pbkdf algorithm");
    auto decryptionKey = PBKDF2_HMAC_SHA_512(password,
                                             to_binary_buf(parameters.passphrase->salt),
                                             parameters.passphrase->iterations,
                                             parameters.passphrase->bits / 8);

    // verify key
    using namespace mtx::crypto;
    auto testKeys = HKDF_SHA256(decryptionKey, BinaryBuf(32, 0), BinaryBuf{});

    auto encrypted = AES_CTR_256_Encrypt(
      std::string(32, '\0'), testKeys.aes, to_binary_buf(base642bin(parameters.iv)));

    auto mac = HMAC_SHA256(testKeys.mac, encrypted);
    if (mac != to_binary_buf(base642bin(parameters.mac))) {
        mtx::utils::log::log()->debug(
          "mac mismatch: {} != {}", bin2base64(to_string(mac)), parameters.mac);
        return std::nullopt;
    }

    return decryptionKey;
}

std::optional<BinaryBuf>
key_from_recoverykey(const std::string &recoverykey,
                     const mtx::secret_storage::AesHmacSha2KeyDescription &parameters)
{
    auto tempKey = to_binary_buf(base582bin(recoverykey));

    if (tempKey.size() < 3 || tempKey[0] != 0x8b || tempKey[1] != 0x01)
        return std::nullopt;

    uint8_t parity = 0;
    for (auto byte = tempKey.begin(); byte != tempKey.end() - 1; ++byte)
        parity ^= *byte;

    if (parity != tempKey.back())
        return std::nullopt;

    auto decryptionKey = BinaryBuf(tempKey.begin() + 2, tempKey.end() - 1);

    // verify key
    using namespace mtx::crypto;
    auto testKeys = HKDF_SHA256(decryptionKey, BinaryBuf(32, 0), BinaryBuf{});

    auto encrypted = AES_CTR_256_Encrypt(
      std::string(32, '\0'), testKeys.aes, to_binary_buf(base642bin(parameters.iv)));

    auto mac = HMAC_SHA256(testKeys.mac, encrypted);
    if (mac != to_binary_buf(base642bin(parameters.mac))) {
        mtx::utils::log::log()->debug(
          "mac mismatch: {} != {}", bin2base64(to_string(mac)), parameters.mac);
        return std::nullopt;
    }

    return decryptionKey;
}

std::string
key_to_recoverykey(const BinaryBuf &key)
{
    auto buf = BinaryBuf(key.size() + 3);
    buf[0]   = 0x8b;
    buf[1]   = 0x01;
    std::copy(begin(key), end(key), begin(buf) + 2);

    uint8_t parity = buf[0] ^ buf[1];
    for (uint8_t b : key)
        parity ^= b;
    buf.back() = parity;

    return bin2base58(to_string(buf));
}

std::string
decrypt(const mtx::secret_storage::AesHmacSha2EncryptedData &data,
        const BinaryBuf &decryptionKey,
        const std::string &key_name)
{
    auto keys   = HKDF_SHA256(decryptionKey, BinaryBuf(32, 0), to_binary_buf(key_name));
    auto keyMac = HMAC_SHA256(keys.mac, to_binary_buf(base642bin(data.ciphertext)));

    if (keyMac != to_binary_buf(base642bin(data.mac))) {
        mtx::utils::log::log()->debug(
          "mac mismatch: {} != {}", bin2base64(to_string(keyMac)), data.mac);
        return "";
    }

    auto decryptedSecret = AES_CTR_256_Decrypt(
      base642bin(data.ciphertext), keys.aes, to_binary_buf(base642bin(data.iv)));

    return to_string(decryptedSecret);
}

mtx::secret_storage::AesHmacSha2EncryptedData
encrypt(const std::string &data, const BinaryBuf &decryptionKey, const std::string &key_name)
{
    mtx::secret_storage::AesHmacSha2EncryptedData encrypted{};
    auto iv      = compatible_iv(create_buffer(16));
    encrypted.iv = bin2base64(to_string(iv));

    auto keys = HKDF_SHA256(decryptionKey, BinaryBuf(32, 0), to_binary_buf(key_name));

    auto ciphertext      = AES_CTR_256_Encrypt(data, keys.aes, iv);
    encrypted.ciphertext = bin2base64(to_string(ciphertext));
    encrypted.mac        = bin2base64(to_string(HMAC_SHA256(keys.mac, ciphertext)));

    return encrypted;
}

HkdfKeys
HKDF_SHA256(const BinaryBuf &key, const BinaryBuf &salt, const BinaryBuf &info)
{
    BinaryBuf buf(64);
    EVP_PKEY_CTX *pctx = EVP_PKEY_CTX_new_id(EVP_PKEY_HKDF, nullptr);

    if (EVP_PKEY_derive_init(pctx) <= 0) {
        EVP_PKEY_CTX_free(pctx);
        throw std::runtime_error("HKDF: failed derive init");
    }
    if (EVP_PKEY_CTX_set_hkdf_md(pctx, EVP_sha256()) <= 0) {
        EVP_PKEY_CTX_free(pctx);
        throw std::runtime_error("HKDF: failed to set digest");
    }
    if (EVP_PKEY_CTX_set1_hkdf_salt(pctx, salt.data(), (int)salt.size()) <= 0) {
        EVP_PKEY_CTX_free(pctx);
        throw std::runtime_error("HKDF: failed to set salt");
    }
    if (EVP_PKEY_CTX_set1_hkdf_key(pctx, key.data(), (int)key.size()) <= 0) {
        EVP_PKEY_CTX_free(pctx);
        throw std::runtime_error("HKDF: failed to set key");
    }
    if (EVP_PKEY_CTX_add1_hkdf_info(pctx, info.data(), (int)info.size()) <= 0) {
        EVP_PKEY_CTX_free(pctx);
        throw std::runtime_error("HKDF: failed to set info");
    }

    std::size_t outlen = buf.size();
    if (EVP_PKEY_derive(pctx, buf.data(), &outlen) <= 0) {
        EVP_PKEY_CTX_free(pctx);
        throw std::runtime_error("HKDF: failed derive");
    }

    EVP_PKEY_CTX_free(pctx);

    if (outlen != 64)
        throw std::runtime_error("Invalid HKDF size!");

    BinaryBuf macKey(buf.begin() + 32, buf.end());
    buf.resize(32);

    return {std::move(buf), std::move(macKey)};
}

BinaryBuf
compatible_iv(BinaryBuf incompatible_iv)
{
    // need to set bit 63 to 0
    // Element and everyone else seems to be counting bytes from the back, i.e. iv_data[15] is
    // the last byte. So we need to clear byte 15 - 63%8 = 15 - 7 = 8, the highest bit, 1 << 7
    // see:
    // https://github.com/matrix-org/matrix-js-sdk/blob/529fe93ab14b93c515e9ab0d0277c1942a5d73c5/src/crypto/aes.ts#L144
    uint8_t *data               = incompatible_iv.data();
    constexpr std::uint8_t mask = static_cast<std::uint8_t>(~(1U << (63 / 8)));
    data[15 - 63 % 8] &= mask;
    return incompatible_iv;
}

BinaryBuf
AES_CTR_256_Encrypt(const std::string &plaintext, const BinaryBuf &aes256Key, BinaryBuf iv)
{
    EVP_CIPHER_CTX *ctx;

    int len;

    int ciphertext_len;

    // The ciphertext expand up to block size, which is 128 for AES256
    BinaryBuf encrypted = compatible_iv(create_buffer(plaintext.size() + AES_BLOCK_SIZE));

    /* Create and initialise the context */
    if (ctx = EVP_CIPHER_CTX_new(); !ctx) {
        // handleErrors();
    }

    if (1 != EVP_EncryptInit_ex(ctx, EVP_aes_256_ctr(), nullptr, aes256Key.data(), iv.data())) {
        // handleErrors();
    }

    /* Provide the message to be encrypted, and obtain the encrypted output.
     * EVP_EncryptUpdate can be called multiple times if necessary
     */
    if (1 != EVP_EncryptUpdate(ctx,
                               encrypted.data(),
                               &len,
                               reinterpret_cast<const unsigned char *>(&plaintext.c_str()[0]),
                               (int)plaintext.size())) {
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
AES_CTR_256_Decrypt(const std::string &ciphertext, const BinaryBuf &aes256Key, BinaryBuf iv)
{
    EVP_CIPHER_CTX *ctx;

    int len;

    int plaintext_len;

    BinaryBuf decrypted = create_buffer(ciphertext.size());

    /* Create and initialise the context */
    if (ctx = EVP_CIPHER_CTX_new(); !ctx) {
        // handleErrors();
    }

    /* Initialise the decryption operation. IMPORTANT - ensure you use a key
     * and IV size appropriate for your cipher
     * In this example we are using 256 bit AES (i.e. a 256 bit key). The
     * IV size for *most* modes is the same as the block size. For AES this
     * is 128 bits */
    if (1 != EVP_DecryptInit_ex(ctx, EVP_aes_256_ctr(), nullptr, aes256Key.data(), iv.data())) {
        // handleErrors();
    }

    /* Provide the message to be decrypted, and obtain the plaintext output.
     * EVP_DecryptUpdate can be called multiple times if necessary
     */
    if (1 != EVP_DecryptUpdate(ctx,
                               decrypted.data(),
                               &len,
                               reinterpret_cast<const unsigned char *>(&ciphertext.data()[0]),
                               (int)ciphertext.size())) {
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
CURVE25519_public_key_from_private(const BinaryBuf &privateKey)
{
    auto ctx = create_olm_object<PkDecryptionObject>();

    BinaryBuf pubkey(::olm_pk_key_length());

    ::olm_pk_key_from_private(
      ctx.get(), pubkey.data(), pubkey.size(), privateKey.data(), privateKey.size());

    return to_string(pubkey);
}

CURVE25519_AES_SHA2_Encrypted
CURVE25519_AES_SHA2_Encrypt(const std::string &plaintext, const std::string &base64_publicKey)
{
    auto ctx = create_olm_object<PkEncryptionObject>();

    ::olm_pk_encryption_set_recipient_key(
      ctx.get(), base64_publicKey.data(), base64_publicKey.size());

    BinaryBuf ephemeral(::olm_pk_key_length());
    BinaryBuf mac(::olm_pk_mac_length(ctx.get()));
    BinaryBuf ciphertext(::olm_pk_ciphertext_length(ctx.get(), plaintext.size()));
    BinaryBuf randomBuf = create_buffer(::olm_pk_encrypt_random_length(ctx.get()));
    auto encrypted_size = ::olm_pk_encrypt(ctx.get(),
                                           plaintext.data(),
                                           plaintext.size(),
                                           ciphertext.data(),
                                           ciphertext.size(),
                                           mac.data(),
                                           mac.size(),
                                           ephemeral.data(),
                                           ephemeral.size(),
                                           randomBuf.data(),
                                           randomBuf.size());

    if (encrypted_size != olm_error()) {
        CURVE25519_AES_SHA2_Encrypted val;
        val.ciphertext = to_string(ciphertext);
        val.mac        = to_string(mac);
        val.ephemeral  = to_string(ephemeral);
        return val;
    } else
        throw olm_exception(__func__, ctx.get());
}

std::string
CURVE25519_AES_SHA2_Decrypt(std::string base64_ciphertext,
                            const BinaryBuf &privateKey,
                            const std::string &ephemeral,
                            const std::string &mac)
{
    auto ctx = create_olm_object<PkDecryptionObject>();

    BinaryBuf pubkey(::olm_pk_key_length());

    ::olm_pk_key_from_private(
      ctx.get(), pubkey.data(), pubkey.size(), privateKey.data(), privateKey.size());

    std::string plaintext(olm_pk_max_plaintext_length(ctx.get(), base64_ciphertext.size()), '\0');
    std::size_t decrypted_size = ::olm_pk_decrypt(ctx.get(),
                                                  ephemeral.data(),
                                                  ephemeral.size(),
                                                  mac.data(),
                                                  mac.size(),
                                                  base64_ciphertext.data(),
                                                  base64_ciphertext.size(),
                                                  plaintext.data(),
                                                  plaintext.size());

    if (decrypted_size != olm_error()) {
        plaintext.resize(decrypted_size);
        return plaintext;
    } else
        throw olm_exception(__func__, ctx.get());
}

mtx::responses::backup::EncryptedSessionData
encrypt_session(const mtx::responses::backup::SessionData &data, const std::string &publicKey)
{
    mtx::responses::backup::EncryptedSessionData d;

    auto temp    = CURVE25519_AES_SHA2_Encrypt(nlohmann::json(data).dump(), publicKey);
    d.ciphertext = std::move(temp.ciphertext);
    d.mac        = std::move(temp.mac);
    d.ephemeral  = std::move(temp.ephemeral);

    return d;
}

mtx::responses::backup::SessionData
decrypt_session(const mtx::responses::backup::EncryptedSessionData &data,
                const BinaryBuf &privateKey)
{
    return nlohmann::json::parse(
             CURVE25519_AES_SHA2_Decrypt(data.ciphertext, privateKey, data.ephemeral, data.mac))
      .get<mtx::responses::backup::SessionData>();
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

    if (context != nullptr) {
        if (EVP_DigestInit_ex(context, EVP_sha256(), nullptr)) {
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

    return AES_CTR_256_Decrypt(ciphertext,
                               to_binary_buf(base642bin_urlsafe_unpadded(encryption_info.key.k)),
                               to_binary_buf(base642bin_unpadded(encryption_info.iv)));
}

std::pair<BinaryBuf, mtx::crypto::EncryptedFile>
encrypt_file(const std::string &plaintext)
{
    mtx::crypto::EncryptedFile encryption_info;

    // iv has to be 16 bytes, key 32!
    BinaryBuf key               = create_buffer(32);
    BinaryBuf iv                = create_buffer(16);
    constexpr std::uint8_t mask = static_cast<std::uint8_t>(~(1U << (63 / 8)));
    iv[15 - 63 % 8] &= mask;

    // Counter should be 0 in v1.1 of the spec...
    for (std::size_t i = 8; i < 16; i++)
        iv[i] = 0;

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
HMAC_SHA256(const BinaryBuf &hmacKey, const BinaryBuf &data)
{
    unsigned int len = SHA256_DIGEST_LENGTH;
    unsigned char digest[SHA256_DIGEST_LENGTH];
    HMAC(EVP_sha256(), hmacKey.data(), (int)hmacKey.size(), data.data(), data.size(), digest, &len);
    BinaryBuf output(digest, digest + SHA256_DIGEST_LENGTH);
    return output;
}

void
uint8_to_uint32(uint8_t b[4], uint32_t &u32)
{
    u32 = std::uint32_t{b[0]} << 24 | std::uint32_t{b[1]} << 16 | std::uint32_t{b[2]} << 8 |
          std::uint32_t{b[3]};
}

void
uint32_to_uint8(uint8_t b[4], uint32_t u32)
{
    b[3] = (uint8_t)u32;
    b[2] = (uint8_t)(u32 >>= 8);
    b[1] = (uint8_t)(u32 >>= 8);
    b[0] = (uint8_t)(u32 >> 8);
}
} // namespace crypto
} // namespace mtx
