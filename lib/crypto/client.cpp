#include <iostream>

#include <nlohmann/json.hpp>

#include <openssl/aes.h>
#include <openssl/sha.h>

#include "mtxclient/crypto/client.hpp"
#include "mtxclient/crypto/types.hpp"
#include "mtxclient/crypto/utils.hpp"

using json = nlohmann::json;
using namespace mtx::crypto;

static constexpr auto pwhash_SALTBYTES = 16u;

using namespace std::string_view_literals;

static const std::array olmErrorStrings{
  "SUCCESS"sv,
  "NOT_ENOUGH_RANDOM"sv,
  "OUTPUT_BUFFER_TOO_SMALL"sv,
  "BAD_MESSAGE_VERSION"sv,
  "BAD_MESSAGE_FORMAT"sv,
  "BAD_MESSAGE_MAC"sv,
  "BAD_MESSAGE_KEY_ID"sv,
  "INVALID_BASE64"sv,
  "BAD_ACCOUNT_KEY"sv,
  "UNKNOWN_PICKLE_VERSION"sv,
  "CORRUPTED_PICKLE"sv,
  "BAD_SESSION_KEY"sv,
  "UNKNOWN_MESSAGE_INDEX"sv,
  "BAD_LEGACY_ACCOUNT_PICKLE"sv,
  "BAD_SIGNATURE"sv,
  "OLM_INPUT_BUFFER_TOO_SMALL"sv,
  "OLM_SAS_THEIR_KEY_NOT_SET"sv,

};

OlmErrorCode
olm_exception::ec_from_string(std::string_view error)
{
    for (size_t i = 0; i < olmErrorStrings.size(); i++) {
        if (olmErrorStrings[i] == error)
            return static_cast<OlmErrorCode>(i);
    }

    return OlmErrorCode::UNKNOWN_ERROR;
}

void
OlmClient::create_new_account()
{
    account_ = create_olm_object<AccountObject>();

    auto tmp_buf = create_buffer(olm_create_account_random_length(account_.get()));
    auto ret     = olm_create_account(account_.get(), tmp_buf.data(), tmp_buf.size());

    if (ret == olm_error())
        throw olm_exception("create_new_account", account_.get());
}

void
OlmClient::restore_account(const std::string &saved_data, const std::string &key)
{
    account_ = unpickle<AccountObject>(saved_data, key);
}

mtx::crypto::IdentityKeys
OlmClient::identity_keys() const
{
    auto tmp_buf = create_buffer(olm_account_identity_keys_length(account_.get()));
    auto ret = olm_account_identity_keys(account_.get(), (void *)tmp_buf.data(), tmp_buf.size());

    if (ret == olm_error())
        throw olm_exception("identity_keys", account_.get());

    return json::parse(std::string(tmp_buf.begin(), tmp_buf.end()));
}

std::string
OlmClient::sign_message(const std::string &msg) const
{
    auto signature_buf = create_buffer(olm_account_signature_length(account_.get()));
    olm_account_sign(
      account_.get(), msg.data(), msg.size(), signature_buf.data(), signature_buf.size());

    return std::string(signature_buf.begin(), signature_buf.end());
}

std::string
OlmClient::sign_identity_keys()
{
    auto keys = identity_keys();

    json body{{"algorithms", {"m.olm.v1.curve25519-aes-sha2", "m.megolm.v1.aes-sha2"}},
              {"user_id", user_id_},
              {"device_id", device_id_},
              {"keys",
               {
                 {"curve25519:" + device_id_, keys.curve25519},
                 {"ed25519:" + device_id_, keys.ed25519},
               }}};

    return sign_message(body.dump());
}

std::size_t
OlmClient::generate_one_time_keys(std::size_t number_of_keys)
{
    const std::size_t nbytes =
      olm_account_generate_one_time_keys_random_length(account_.get(), number_of_keys);

    auto buf = create_buffer(nbytes);

    auto ret =
      olm_account_generate_one_time_keys(account_.get(), number_of_keys, buf.data(), buf.size());

    if (ret == olm_error())
        throw olm_exception("generate_one_time_keys", account_.get());

    return ret;
}

mtx::crypto::OneTimeKeys
OlmClient::one_time_keys()
{
    auto buf = create_buffer(olm_account_one_time_keys_length(account_.get()));

    const auto ret = olm_account_one_time_keys(account_.get(), buf.data(), buf.size());

    if (ret == olm_error())
        throw olm_exception("one_time_keys", account_.get());

    return json::parse(std::string(buf.begin(), buf.end()));
}

std::string
OlmClient::sign_one_time_key(const std::string &key)
{
    json j{{"key", key}};
    return sign_message(j.dump());
}

std::map<std::string, mtx::requests::SignedOneTimeKey>
OlmClient::sign_one_time_keys(const OneTimeKeys &keys)
{
    // Sign & append the one time keys.
    std::map<std::string, mtx::requests::SignedOneTimeKey> signed_one_time_keys;
    for (const auto &elem : keys.curve25519) {
        const auto key_id       = elem.first;
        const auto one_time_key = elem.second;

        auto sig = sign_one_time_key(one_time_key);

        signed_one_time_keys["signed_curve25519:" + key_id] =
          signed_one_time_key(one_time_key, sig);
    }

    return signed_one_time_keys;
}

mtx::requests::SignedOneTimeKey
OlmClient::signed_one_time_key(const std::string &key, const std::string &signature)
{
    mtx::requests::SignedOneTimeKey sign{};
    sign.key        = key;
    sign.signatures = {{user_id_, {{"ed25519:" + device_id_, signature}}}};
    return sign;
}

mtx::requests::UploadKeys
OlmClient::create_upload_keys_request()
{
    return create_upload_keys_request(one_time_keys());
}

mtx::requests::UploadKeys
OlmClient::create_upload_keys_request(const mtx::crypto::OneTimeKeys &one_time_keys)
{
    mtx::requests::UploadKeys req;
    req.device_keys.user_id   = user_id_;
    req.device_keys.device_id = device_id_;

    auto id_keys = identity_keys();

    req.device_keys.keys["curve25519:" + device_id_] = id_keys.curve25519;
    req.device_keys.keys["ed25519:" + device_id_]    = id_keys.ed25519;

    // Generate and add the signature to the request.
    auto sig = sign_identity_keys();

    req.device_keys.signatures[user_id_]["ed25519:" + device_id_] = sig;

    if (one_time_keys.curve25519.empty())
        return req;

    // Sign & append the one time keys.
    auto temp = sign_one_time_keys(one_time_keys);
    for (const auto &[key_id, key] : temp)
        req.one_time_keys[key_id] = key;

    return req;
}

OutboundGroupSessionPtr
OlmClient::init_outbound_group_session()
{
    auto session = create_olm_object<OutboundSessionObject>();
    auto tmp_buf = create_buffer(olm_init_outbound_group_session_random_length(session.get()));

    const auto ret = olm_init_outbound_group_session(session.get(), tmp_buf.data(), tmp_buf.size());

    if (ret == olm_error())
        throw olm_exception("init_outbound_group_session", session.get());

    return session;
}

InboundGroupSessionPtr
OlmClient::init_inbound_group_session(const std::string &session_key)
{
    auto session = create_olm_object<InboundSessionObject>();

    auto temp      = session_key;
    const auto ret = olm_init_inbound_group_session(
      session.get(), reinterpret_cast<const uint8_t *>(temp.data()), temp.size());

    if (ret == olm_error())
        throw olm_exception("init_inbound_group_session", session.get());

    return session;
}

InboundGroupSessionPtr
OlmClient::import_inbound_group_session(const std::string &session_key)
{
    auto session = create_olm_object<InboundSessionObject>();

    auto temp      = session_key;
    const auto ret = olm_import_inbound_group_session(
      session.get(), reinterpret_cast<const uint8_t *>(temp.data()), temp.size());

    if (ret == olm_error())
        throw olm_exception("init_inbound_group_session", session.get());

    return session;
}

GroupPlaintext
OlmClient::decrypt_group_message(OlmInboundGroupSession *session,
                                 const std::string &message,
                                 uint32_t message_index)
{
    // TODO handle errors
    auto tmp_msg = create_buffer(message.size());
    std::copy(message.begin(), message.end(), tmp_msg.begin());

    auto plaintext_len =
      olm_group_decrypt_max_plaintext_length(session, tmp_msg.data(), tmp_msg.size());
    auto plaintext = create_buffer(plaintext_len);

    tmp_msg = create_buffer(message.size());
    std::copy(message.begin(), message.end(), tmp_msg.begin());

    const std::size_t nbytes = olm_group_decrypt(
      session, tmp_msg.data(), tmp_msg.size(), plaintext.data(), plaintext.size(), &message_index);

    if (nbytes == olm_error())
        throw olm_exception("olm_group_decrypt", session);

    auto output = create_buffer(nbytes);
    std::memcpy(output.data(), plaintext.data(), nbytes);

    return GroupPlaintext{std::move(output), message_index};
}

BinaryBuf
OlmClient::encrypt_group_message(OlmOutboundGroupSession *session, const std::string &plaintext)
{
    auto encrypted_len     = olm_group_encrypt_message_length(session, plaintext.size());
    auto encrypted_message = create_buffer(encrypted_len);

    const std::size_t nbytes =
      olm_group_encrypt(session,
                        reinterpret_cast<const uint8_t *>(plaintext.data()),
                        plaintext.size(),
                        encrypted_message.data(),
                        encrypted_message.size());

    if (nbytes == olm_error())
        throw olm_exception("olm_group_encrypt", session);

    return encrypted_message;
}

BinaryBuf
OlmClient::decrypt_message(OlmSession *session,
                           size_t msgtype,
                           const std::string &one_time_key_message)
{
    auto tmp = create_buffer(one_time_key_message.size());
    std::copy(one_time_key_message.begin(), one_time_key_message.end(), tmp.begin());

    auto declen =
      olm_decrypt_max_plaintext_length(session, msgtype, (void *)tmp.data(), tmp.size());

    auto decrypted = create_buffer(declen);
    std::copy(one_time_key_message.begin(), one_time_key_message.end(), tmp.begin());

    const std::size_t nbytes = olm_decrypt(
      session, msgtype, (void *)tmp.data(), tmp.size(), decrypted.data(), decrypted.size());

    if (nbytes == olm_error())
        throw olm_exception("olm_decrypt", session);

    // Removing the extra padding from the origial buffer.
    auto output = create_buffer(nbytes);
    std::memcpy(output.data(), decrypted.data(), nbytes);

    return output;
}

BinaryBuf
OlmClient::encrypt_message(OlmSession *session, const std::string &msg)
{
    auto ciphertext = create_buffer(olm_encrypt_message_length(session, msg.size()));
    auto random_buf = create_buffer(olm_encrypt_random_length(session));

    const auto ret = olm_encrypt(session,
                                 msg.data(),
                                 msg.size(),
                                 random_buf.data(),
                                 random_buf.size(),
                                 ciphertext.data(),
                                 ciphertext.size());
    if (ret == olm_error())
        throw olm_exception("olm_encrypt", session);

    return ciphertext;
}

OlmSessionPtr
OlmClient::create_inbound_session_from(const std::string &their_curve25519,
                                       const std::string &one_time_key_message)
{
    BinaryBuf tmp(one_time_key_message.size());
    memcpy(tmp.data(), one_time_key_message.data(), one_time_key_message.size());

    return create_inbound_session_from(std::move(their_curve25519), std::move(tmp));
}

OlmSessionPtr
OlmClient::create_inbound_session_from(const std::string &their_curve25519,
                                       const BinaryBuf &one_time_key_message)
{
    auto session = create_olm_object<SessionObject>();

    auto tmp = create_buffer(one_time_key_message.size());
    std::copy(one_time_key_message.begin(), one_time_key_message.end(), tmp.begin());

    std::size_t ret = olm_create_inbound_session_from(session.get(),
                                                      account(),
                                                      their_curve25519.data(),
                                                      their_curve25519.size(),
                                                      (void *)tmp.data(),
                                                      tmp.size());

    if (ret == olm_error())
        throw olm_exception("create_inbound_session_from", session.get());

    ret = olm_remove_one_time_keys(account_.get(), session.get());

    if (ret == olm_error())
        throw olm_exception("inbound_session_from_remove_one_time_keys", account_.get());

    return session;
}

OlmSessionPtr
OlmClient::create_inbound_session(const std::string &one_time_key_message)
{
    BinaryBuf tmp(one_time_key_message.size());
    memcpy(tmp.data(), one_time_key_message.data(), one_time_key_message.size());

    return create_inbound_session(std::move(tmp));
}

OlmSessionPtr
OlmClient::create_inbound_session(const BinaryBuf &one_time_key_message)
{
    auto session = create_olm_object<SessionObject>();

    auto tmp = create_buffer(one_time_key_message.size());
    std::copy(one_time_key_message.begin(), one_time_key_message.end(), tmp.begin());

    std::size_t ret =
      olm_create_inbound_session(session.get(), account(), (void *)tmp.data(), tmp.size());

    if (ret == olm_error())
        throw olm_exception("create_inbound_session", session.get());

    ret = olm_remove_one_time_keys(account_.get(), session.get());

    if (ret == olm_error())
        throw olm_exception("inbound_session_remove_one_time_keys", account_.get());

    return session;
}

OlmSessionPtr
OlmClient::create_outbound_session(const std::string &identity_key, const std::string &one_time_key)
{
    auto session    = create_olm_object<SessionObject>();
    auto random_buf = create_buffer(olm_create_outbound_session_random_length(session.get()));

    const auto ret = olm_create_outbound_session(session.get(),
                                                 account(),
                                                 identity_key.data(),
                                                 identity_key.size(),
                                                 one_time_key.data(),
                                                 one_time_key.size(),
                                                 random_buf.data(),
                                                 random_buf.size());

    if (ret == olm_error())
        throw olm_exception("create_outbound_session", session.get());

    return session;
}

std::unique_ptr<SAS>
OlmClient::sas_init()
{
    return std::make_unique<SAS>();
}

//! constructor which create a new Curve25519 key pair which is stored in SASObject
SAS::SAS()
{
    this->sas       = create_olm_object<SASObject>();
    auto random_buf = BinaryBuf(olm_create_sas_random_length(sas.get()));

    const auto ret = olm_create_sas(this->sas.get(), random_buf.data(), random_buf.size());

    if (ret == olm_error())
        throw olm_exception("create_sas_instance", this->sas.get());
}

std::string
SAS::public_key()
{
    auto pub_key_buffer = create_buffer(olm_sas_pubkey_length(this->sas.get()));

    const auto ret =
      olm_sas_get_pubkey(this->sas.get(), pub_key_buffer.data(), pub_key_buffer.size());

    if (ret == olm_error())
        throw olm_exception("get_public_key", this->sas.get());

    return to_string(pub_key_buffer);
}

void
SAS::set_their_key(std::string their_public_key)
{
    auto pub_key_buffer = to_binary_buf(their_public_key);

    const auto ret =
      olm_sas_set_their_key(this->sas.get(), pub_key_buffer.data(), pub_key_buffer.size());

    if (ret == olm_error())
        throw olm_exception("get_public_key", this->sas.get());
}

std::vector<int>
SAS::generate_bytes_decimal(std::string info)
{
    auto input_info_buffer = to_binary_buf(info);
    auto output_buffer     = BinaryBuf(5);

    std::vector<int> output_list;
    output_list.resize(3);

    const auto ret = olm_sas_generate_bytes(this->sas.get(),
                                            input_info_buffer.data(),
                                            input_info_buffer.size(),
                                            output_buffer.data(),
                                            output_buffer.size());

    if (ret == olm_error())
        throw olm_exception("get_bytes_decimal", this->sas.get());

    output_list[0] = (((output_buffer[0] << 5) | (output_buffer[1] >> 3)) + 1000);
    output_list[1] =
      ((((output_buffer[1] & 0x07) << 10) | (output_buffer[2] << 2) | (output_buffer[3] >> 6)) +
       1000);
    output_list[2] = (((((output_buffer[3] & 0x3F) << 7)) | ((output_buffer[4] >> 1))) + 1000);

    return output_list;
}

//! generates and returns a vector of number(int) ranging from 0 to 63, to be used only after using
//! `set_their_key`
std::vector<int>
SAS::generate_bytes_emoji(std::string info)
{
    auto input_info_buffer = to_binary_buf(info);
    auto output_buffer     = BinaryBuf(6);

    std::vector<int> output_list;
    output_list.resize(7);

    const auto ret = olm_sas_generate_bytes(this->sas.get(),
                                            input_info_buffer.data(),
                                            input_info_buffer.size(),
                                            output_buffer.data(),
                                            output_buffer.size());

    if (ret == olm_error())
        throw olm_exception("get_bytes_emoji", this->sas.get());

    output_list[0] = (output_buffer[0] >> 2);
    output_list[1] = (((output_buffer[0] << 4) & 0x3f) | (output_buffer[1] >> 4));
    output_list[2] = (((output_buffer[1] << 2) & 0x3f) | (output_buffer[2] >> 6));
    output_list[3] = (output_buffer[2] & 0x3f);
    output_list[4] = (output_buffer[3] >> 2);
    output_list[5] = (((output_buffer[3] << 4) & 0x3f) | (output_buffer[4] >> 4));
    output_list[6] = (((output_buffer[4] << 2) & 0x3f) | (output_buffer[5] >> 6));

    return output_list;
}

//! calculates the mac based on the given input and info using the shared secret produced after
//! `set_their_key`
std::string
SAS::calculate_mac(std::string input_data, std::string info)
{
    auto input_data_buffer = to_binary_buf(input_data);
    auto info_buffer       = to_binary_buf(info);
    auto output_buffer     = BinaryBuf(olm_sas_mac_length(this->sas.get()));

    const auto ret = olm_sas_calculate_mac(this->sas.get(),
                                           input_data_buffer.data(),
                                           input_data_buffer.size(),
                                           info_buffer.data(),
                                           info_buffer.size(),
                                           output_buffer.data(),
                                           output_buffer.size());

    if (ret == olm_error())
        throw olm_exception("calculate_mac", this->sas.get());

    return to_string(output_buffer);
}

PkSigning
PkSigning::from_seed(std::string seed)
{
    PkSigning s{};
    s.signing = create_olm_object<PkSigningObject>();

    auto seed_ = base642bin(seed);

    auto pub_key_buffer = BinaryBuf(olm_pk_signing_public_key_length());
    auto ret            = olm_pk_signing_key_from_seed(
      s.signing.get(), pub_key_buffer.data(), pub_key_buffer.size(), seed_.data(), seed_.size());

    if (ret == olm_error())
        throw olm_exception("signing_from_seed", s.signing.get());

    s.public_key_ = to_string(pub_key_buffer);

    return s;
}

std::string
PkSigning::sign(const std::string &message)
{
    auto signature = BinaryBuf(olm_pk_signature_length());
    auto message_  = to_binary_buf(message);

    auto ret = olm_pk_sign(
      signing.get(), message_.data(), message_.size(), signature.data(), signature.size());

    if (ret == olm_error())
        throw olm_exception("olm_pk_sign", signing.get());

    return to_string(signature);
}

nlohmann::json
OlmClient::create_olm_encrypted_content(OlmSession *session,
                                        nlohmann::json event,
                                        const UserId &recipient,
                                        const std::string &recipient_ed25519_key,
                                        const std::string &recipient_curve25519_key)
{
    event["keys"]["ed25519"] = identity_keys().ed25519;
    event["sender"]          = user_id_;
    event["sender_device"]   = device_id_;

    event["recipient"]                 = recipient.get();
    event["recipient_keys"]["ed25519"] = recipient_ed25519_key;

    size_t msg_type    = olm_encrypt_message_type(session);
    auto encrypted     = encrypt_message(session, json(event).dump());
    auto encrypted_str = std::string((char *)encrypted.data(), encrypted.size());

    return json{
      {"algorithm", "m.olm.v1.curve25519-aes-sha2"},
      {"sender_key", identity_keys().curve25519},
      {"ciphertext", {{recipient_curve25519_key, {{"body", encrypted_str}, {"type", msg_type}}}}}};
}

std::string
OlmClient::save(const std::string &key)
{
    if (!account_)
        return std::string();

    return pickle<AccountObject>(account(), key);
}

std::string
mtx::crypto::session_id(OlmSession *s)
{
    auto tmp = create_buffer(olm_session_id_length(s));
    olm_session_id(s, tmp.data(), tmp.size());

    return std::string(tmp.begin(), tmp.end());
}

std::string
mtx::crypto::session_id(OlmOutboundGroupSession *s)
{
    auto tmp = create_buffer(olm_outbound_group_session_id_length(s));
    olm_outbound_group_session_id(s, tmp.data(), tmp.size());

    return std::string(tmp.begin(), tmp.end());
}

std::string
mtx::crypto::session_key(OlmOutboundGroupSession *s)
{
    auto tmp = create_buffer(olm_outbound_group_session_key_length(s));
    olm_outbound_group_session_key(s, tmp.data(), tmp.size());

    return std::string(tmp.begin(), tmp.end());
}

std::string
mtx::crypto::export_session(OlmInboundGroupSession *s, uint32_t at_index)
{
    const size_t len = olm_export_inbound_group_session_length(s);
    const uint32_t index =
      at_index == uint32_t(-1) ? olm_inbound_group_session_first_known_index(s) : at_index;

    auto session_key = create_buffer(len);
    const std::size_t ret =
      olm_export_inbound_group_session(s, session_key.data(), session_key.size(), index);

    if (ret == olm_error())
        throw olm_exception("session_key", s);

    return std::string(session_key.begin(), session_key.end());
}

InboundGroupSessionPtr
mtx::crypto::import_session(const std::string &session_key)
{
    auto session = create_olm_object<InboundSessionObject>();

    const std::size_t ret = olm_import_inbound_group_session(
      session.get(), reinterpret_cast<const uint8_t *>(session_key.data()), session_key.size());

    if (ret == olm_error())
        throw olm_exception("import_session", session.get());

    return session;
}

bool
mtx::crypto::matches_inbound_session(OlmSession *session, const std::string &one_time_key_message)
{
    auto tmp = create_buffer(one_time_key_message.size());
    std::copy(one_time_key_message.begin(), one_time_key_message.end(), tmp.begin());

    return olm_matches_inbound_session(session, (void *)tmp.data(), tmp.size());
}

bool
mtx::crypto::matches_inbound_session_from(OlmSession *session,
                                          const std::string &id_key,
                                          const std::string &one_time_key_message)
{
    auto tmp = create_buffer(one_time_key_message.size());
    std::copy(one_time_key_message.begin(), one_time_key_message.end(), tmp.begin());

    return olm_matches_inbound_session_from(
      session, id_key.data(), id_key.size(), (void *)tmp.data(), tmp.size());
}

bool
mtx::crypto::verify_identity_signature(const DeviceKeys &device_keys,
                                       const DeviceId &device_id,
                                       const UserId &user_id)
{
    try {
        const auto sign_key_id = "ed25519:" + device_id.get();
        const auto signing_key = device_keys.keys.at(sign_key_id);
        const auto signature   = device_keys.signatures.at(user_id.get()).at(sign_key_id);

        if (signature.empty())
            return false;

        return ed25519_verify_signature(signing_key, nlohmann::json(device_keys), signature);

    } catch (const nlohmann::json::exception &e) {
        std::cerr << "verify_identity_signature: " << e.what();
    }

    return false;
}

//! checks if the signature is signed by the signing_key
bool
mtx::crypto::ed25519_verify_signature(std::string signing_key,
                                      nlohmann::json obj,
                                      std::string signature)
{
    try {
        if (signature.empty())
            return false;

        obj.erase("unsigned");
        obj.erase("signatures");

        std::string canonical_json = obj.dump();

        auto utility = create_olm_object<UtilityObject>();
        auto ret     = olm_ed25519_verify(utility.get(),
                                      signing_key.data(),
                                      signing_key.size(),
                                      canonical_json.data(),
                                      canonical_json.size(),
                                      (void *)signature.data(),
                                      signature.size());

        // the signature is wrong
        if (ret != 0)
            return false;

        return true;
    } catch (const nlohmann::json::exception &e) {
        std::cerr << "verify_signature: " << e.what();
    }

    return false;
}

std::string
mtx::crypto::encrypt_exported_sessions(const mtx::crypto::ExportedSessionKeys &keys,
                                       std::string pass)
{
    const auto plaintext = json(keys).dump();

    auto nonce = create_buffer(AES_BLOCK_SIZE);
    nonce[15 - 63 % 8] &= ~(1UL << (63 / 8));

    auto salt = create_buffer(pwhash_SALTBYTES);

    auto buf = create_buffer(64U);

    uint32_t iterations = 100000;
    buf                 = mtx::crypto::PBKDF2_HMAC_SHA_512(pass, salt, iterations);

    BinaryBuf aes256 = BinaryBuf(buf.begin(), buf.begin() + 32);

    BinaryBuf hmac256 = BinaryBuf(buf.begin() + 32, buf.begin() + (2 * 32));

    auto ciphertext = mtx::crypto::AES_CTR_256_Encrypt(plaintext, aes256, nonce);

    uint8_t iterationsArr[4];
    mtx::crypto::uint32_to_uint8(iterationsArr, iterations);

    // Format of the output buffer: (0x01 + salt + IV + number of rounds + ciphertext +
    // hmac-sha-256)
    BinaryBuf output{0x01};
    output.insert(
      output.end(), std::make_move_iterator(salt.begin()), std::make_move_iterator(salt.end()));
    output.insert(
      output.end(), std::make_move_iterator(nonce.begin()), std::make_move_iterator(nonce.end()));
    output.insert(output.end(), &iterationsArr[0], &iterationsArr[4]);
    output.insert(output.end(),
                  std::make_move_iterator(ciphertext.begin()),
                  std::make_move_iterator(ciphertext.end()));

    // Need to hmac-sha256 our string so far, and then use that to finish making the output.
    auto hmacSha256 = mtx::crypto::HMAC_SHA256(hmac256, output);

    output.insert(output.end(),
                  std::make_move_iterator(hmacSha256.begin()),
                  std::make_move_iterator(hmacSha256.end()));
    auto encrypted = std::string(output.begin(), output.end());

    return encrypted;
}

mtx::crypto::ExportedSessionKeys
mtx::crypto::decrypt_exported_sessions(const std::string &data, std::string pass)
{
    // Parse the data into a base64 string without the header and footer
    std::string unpacked = mtx::crypto::unpack_key_file(data);

    std::string binary_str = base642bin(unpacked);

    if (binary_str.size() <
        1 + pwhash_SALTBYTES + AES_BLOCK_SIZE + sizeof(uint32_t) + SHA256_DIGEST_LENGTH + 2)
        throw crypto_exception("decrypt_exported_sessions", "Invalid session file: too short");

    const auto binary_start = binary_str.begin();
    const auto binary_end   = binary_str.end();

    // Format version 0x01, 1 byte
    const auto format_end = binary_start + 1;
    auto format           = BinaryBuf(binary_start, format_end);
    if (format[0] != 0x01)
        throw crypto_exception("decrypt_exported_sessions", "Unsupported backup file format.");

    // Salt, 16 bytes
    const auto salt_end = format_end + pwhash_SALTBYTES;
    auto salt           = BinaryBuf(format_end, salt_end);

    // IV, 16 bytes
    const auto iv_end = salt_end + AES_BLOCK_SIZE;
    auto iv           = BinaryBuf(salt_end, iv_end);

    // Number of rounds, 4 bytes
    const auto rounds_end = iv_end + sizeof(uint32_t);
    auto rounds_buff      = BinaryBuf(iv_end, rounds_end);
    uint8_t rounds_arr[4];
    std::copy(rounds_buff.begin(), rounds_buff.end(), rounds_arr);
    uint32_t rounds;
    mtx::crypto::uint8_to_uint32(rounds_arr, rounds);

    // Variable-length JSON object...
    const auto json_end = binary_end - SHA256_DIGEST_LENGTH;
    auto json           = BinaryBuf(rounds_end, json_end);

    // HMAC of the above, 32 bytes
    auto hmac = BinaryBuf(json_end, binary_end);

    // derive the keys
    auto buf = mtx::crypto::PBKDF2_HMAC_SHA_512(pass, salt, rounds);

    BinaryBuf aes256 = BinaryBuf(buf.begin(), buf.begin() + 32);

    BinaryBuf hmac256 = BinaryBuf(buf.begin() + 32, buf.begin() + (2 * 32));

    // get hmac and verify they match
    auto hmacSha256 = mtx::crypto::HMAC_SHA256(hmac256, BinaryBuf(binary_start, json_end));

    if (hmacSha256 != hmac) {
        throw crypto_exception{"decrypt_exported_sessions", "HMAC doesn't match"};
    }

    const std::string ciphertext(json.begin(), json.end());
    auto decrypted = mtx::crypto::AES_CTR_256_Decrypt(ciphertext, aes256, iv);

    std::string plaintext(decrypted.begin(), decrypted.end());
    return json::parse(plaintext);
}
