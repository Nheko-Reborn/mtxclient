#include <iostream>

#include "mtxclient/crypto/client.hpp"
#include "mtxclient/crypto/types.hpp"

#include "sodium.h"

using json = nlohmann::json;
using namespace mtx::crypto;

void
OlmClient::create_new_account()
{
        account_ = create_olm_object<AccountObject>();

        auto tmp_buf  = create_buffer(olm_create_account_random_length(account_.get()));
        const int ret = olm_create_account(account_.get(), tmp_buf.data(), tmp_buf.size());

        if (ret == -1)
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
        int result =
          olm_account_identity_keys(account_.get(), (void *)tmp_buf.data(), tmp_buf.size());

        if (result == -1)
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

        const int ret = olm_account_generate_one_time_keys(
          account_.get(), number_of_keys, buf.data(), buf.size());

        if (ret == -1)
                throw olm_exception("generate_one_time_keys", account_.get());

        return ret;
}

mtx::crypto::OneTimeKeys
OlmClient::one_time_keys()
{
        auto buf = create_buffer(olm_account_one_time_keys_length(account_.get()));

        const int ret = olm_account_one_time_keys(account_.get(), buf.data(), buf.size());

        if (ret == -1)
                throw olm_exception("one_time_keys", account_.get());

        return json::parse(std::string(buf.begin(), buf.end()));
}

std::string
OlmClient::sign_one_time_key(const std::string &key)
{
        json j{{"key", key}};
        return sign_message(j.dump());
}

std::map<std::string, json>
OlmClient::sign_one_time_keys(const OneTimeKeys &keys)
{
        // Sign & append the one time keys.
        std::map<std::string, json> signed_one_time_keys;
        for (const auto &elem : keys.curve25519) {
                const auto key_id       = elem.first;
                const auto one_time_key = elem.second;

                auto sig = sign_one_time_key(one_time_key);

                signed_one_time_keys["signed_curve25519:" + key_id] =
                  signed_one_time_key_json(one_time_key, sig);
        }

        return signed_one_time_keys;
}

json
OlmClient::signed_one_time_key_json(const std::string &key, const std::string &signature)
{
        return json{{"key", key},
                    {"signatures", {{user_id_, {{"ed25519:" + device_id_, signature}}}}}};
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
        req.one_time_keys = sign_one_time_keys(one_time_keys);

        return req;
}

OutboundGroupSessionPtr
OlmClient::init_outbound_group_session()
{
        auto session = create_olm_object<OutboundSessionObject>();
        auto tmp_buf = create_buffer(olm_init_outbound_group_session_random_length(session.get()));

        const int ret =
          olm_init_outbound_group_session(session.get(), tmp_buf.data(), tmp_buf.size());

        if (ret == -1)
                throw olm_exception("init_outbound_group_session", session.get());

        return session;
}

InboundGroupSessionPtr
OlmClient::init_inbound_group_session(const std::string &session_key)
{
        auto session = create_olm_object<InboundSessionObject>();

        const int ret = olm_init_inbound_group_session(
          session.get(), reinterpret_cast<const uint8_t *>(session_key.data()), session_key.size());

        if (ret == -1)
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

        const int nbytes = olm_group_decrypt(session,
                                             tmp_msg.data(),
                                             tmp_msg.size(),
                                             plaintext.data(),
                                             plaintext.size(),
                                             &message_index);

        if (nbytes == -1)
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

        const int nbytes = olm_group_encrypt(session,
                                             reinterpret_cast<const uint8_t *>(plaintext.data()),
                                             plaintext.size(),
                                             encrypted_message.data(),
                                             encrypted_message.size());

        if (nbytes == -1)
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

        const int nbytes = olm_decrypt(
          session, msgtype, (void *)tmp.data(), tmp.size(), decrypted.data(), decrypted.size());

        if (nbytes == -1)
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

        const int ret = olm_encrypt(session,
                                    msg.data(),
                                    msg.size(),
                                    random_buf.data(),
                                    random_buf.size(),
                                    ciphertext.data(),
                                    ciphertext.size());
        if (ret == -1)
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

        int ret = olm_create_inbound_session_from(session.get(),
                                                  account(),
                                                  their_curve25519.data(),
                                                  their_curve25519.size(),
                                                  (void *)tmp.data(),
                                                  tmp.size());

        if (ret == -1)
                throw olm_exception("create_inbound_session_from", session.get());

        ret = olm_remove_one_time_keys(account_.get(), session.get());

        if (ret == -1)
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

        int ret =
          olm_create_inbound_session(session.get(), account(), (void *)tmp.data(), tmp.size());

        if (ret == -1)
                throw olm_exception("create_inbound_session", session.get());

        ret = olm_remove_one_time_keys(account_.get(), session.get());

        if (ret == -1)
                throw olm_exception("inbound_session_remove_one_time_keys", account_.get());

        return session;
}

OlmSessionPtr
OlmClient::create_outbound_session(const std::string &identity_key, const std::string &one_time_key)
{
        auto session    = create_olm_object<SessionObject>();
        auto random_buf = create_buffer(olm_create_outbound_session_random_length(session.get()));

        const int ret = olm_create_outbound_session(session.get(),
                                                    account(),
                                                    identity_key.data(),
                                                    identity_key.size(),
                                                    one_time_key.data(),
                                                    one_time_key.size(),
                                                    random_buf.data(),
                                                    random_buf.size());

        if (ret == -1)
                throw olm_exception("create_outbound_session", session.get());

        return session;
}

nlohmann::json
OlmClient::create_room_key_event(const UserId &recipient,
                                 const std::string &ed25519_recipient_key,
                                 const nlohmann::json &content) const noexcept
{
        return json{{"content", content},
                    {"keys", {{"ed25519", identity_keys().ed25519}}},
                    {"recipient", recipient.get()},
                    {"recipient_keys", {{"ed25519", ed25519_recipient_key}}},
                    {"sender", user_id_},
                    {"sender_device", device_id_},
                    {"type", "m.room_key"}};
}

nlohmann::json
OlmClient::create_olm_encrypted_content(OlmSession *session,
                                        const std::string &room_key_event,
                                        const std::string &recipient_key)
{
        size_t msg_type    = olm_encrypt_message_type(session);
        auto encrypted     = encrypt_message(session, room_key_event);
        auto encrypted_str = std::string((char *)encrypted.data(), encrypted.size());

        return json{
          {"algorithm", "m.olm.v1.curve25519-aes-sha2"},
          {"sender_key", identity_keys().curve25519},
          {"ciphertext", {{recipient_key, {{"body", encrypted_str}, {"type", msg_type}}}}}};
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
mtx::crypto::export_session(OlmInboundGroupSession *s)
{
        const size_t len     = olm_export_inbound_group_session_length(s);
        const uint32_t index = olm_inbound_group_session_first_known_index(s);

        auto session_key = create_buffer(len);
        const int rc =
          olm_export_inbound_group_session(s, session_key.data(), session_key.size(), index);

        if (rc == -1)
                throw olm_exception("session_key", s);

        return std::string(session_key.begin(), session_key.end());
}

InboundGroupSessionPtr
mtx::crypto::import_session(const std::string &session_key)
{
        auto session = create_olm_object<InboundSessionObject>();

        const int rc = olm_import_inbound_group_session(
          session.get(), reinterpret_cast<const uint8_t *>(session_key.data()), session_key.size());

        if (rc == -1)
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
mtx::crypto::verify_identity_signature(nlohmann::json obj,
                                       const DeviceId &device_id,
                                       const UserId &user_id)
{
        using namespace client::utils;

        try {
                const auto sign_key_id = "ed25519:" + device_id.get();
                const auto signing_key = obj.at("keys").at(sign_key_id).get<std::string>();
                const auto signature =
                  obj.at("signatures").at(user_id.get()).at(sign_key_id).get<std::string>();

                if (signature.empty())
                        return false;

                obj.erase("unsigned");
                obj.erase("signatures");

                const auto msg = obj.dump();

                auto utility = create_olm_object<UtilityObject>();
                auto ret     = olm_ed25519_verify(utility.get(),
                                              signing_key.data(),
                                              signing_key.size(),
                                              msg.data(),
                                              msg.size(),
                                              (void *)signature.data(),
                                              signature.size());

                if (ret != 0)
                        throw olm_exception("verify_identity_signature", utility.get());

                return true;
        } catch (const nlohmann::json::exception &e) {
                std::cerr << "verify_identity_signature: " << e.what();
        }

        return false;
}

std::string
mtx::crypto::encrypt_exported_sessions(const mtx::crypto::ExportedSessionKeys &keys,
                                       std::string pass)
{
        const auto plaintext      = json(keys).dump();
        const auto msg_len        = plaintext.size();
        const auto ciphertext_len = crypto_secretbox_MACBYTES + msg_len;

        auto nonce      = create_buffer(crypto_secretbox_NONCEBYTES);
        auto ciphertext = create_buffer(ciphertext_len);

        auto salt = create_buffer(crypto_pwhash_SALTBYTES);
        auto key  = derive_key(pass, salt);

        crypto_secretbox_easy(reinterpret_cast<unsigned char *>(ciphertext.data()),
                              reinterpret_cast<const unsigned char *>(plaintext.data()),
                              msg_len,
                              nonce.data(),
                              reinterpret_cast<const unsigned char *>(key.data()));

        // Format of the output buffer: (nonce + salt + ciphertext)
        BinaryBuf output{nonce};
        output.insert(
          output.end(), std::make_move_iterator(salt.begin()), std::make_move_iterator(salt.end()));
        output.insert(output.end(),
                      std::make_move_iterator(ciphertext.begin()),
                      std::make_move_iterator(ciphertext.end()));

        return std::string(output.begin(), output.end());
}

mtx::crypto::ExportedSessionKeys
mtx::crypto::decrypt_exported_sessions(const std::string &data, std::string pass)
{
        if (data.size() <
            crypto_secretbox_MACBYTES + crypto_secretbox_NONCEBYTES + crypto_pwhash_SALTBYTES)
                throw sodium_exception{"decrypt_exported_sessions", "ciphertext too small"};

        const auto nonce_start = data.begin();
        const auto nonce_end   = nonce_start + crypto_secretbox_NONCEBYTES;
        auto nonce             = BinaryBuf(nonce_start, nonce_end);

        const auto salt_end = nonce_end + crypto_pwhash_SALTBYTES;
        auto salt           = BinaryBuf(nonce_end, salt_end);

        auto ciphertext = BinaryBuf(salt_end, data.end());
        auto decrypted  = create_buffer(ciphertext.size() - crypto_secretbox_MACBYTES);

        auto key = derive_key(pass, salt);

        if (crypto_secretbox_open_easy(decrypted.data(),
                                       reinterpret_cast<const unsigned char *>(ciphertext.data()),
                                       ciphertext.size(),
                                       nonce.data(),
                                       reinterpret_cast<const unsigned char *>(key.data())) != 0)
                throw sodium_exception{"crypto_secretbox_open_easy", "failed to decrypt"};

        return json::parse(std::string(decrypted.begin(), decrypted.end()));
}

std::string
mtx::crypto::base642bin(const std::string &b64)
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
mtx::crypto::bin2base64(const std::string &bin)
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

BinaryBuf
mtx::crypto::derive_key(const std::string &pass, const BinaryBuf &salt)
{
        if (salt.size() != crypto_pwhash_SALTBYTES)
                throw sodium_exception{"derive_key", "invalid buffer size for salt"};

        auto key = create_buffer(crypto_secretbox_KEYBYTES);

        // Derive a key from the user provided password.
        if (crypto_pwhash(key.data(),
                          key.size(),
                          pass.data(),
                          pass.size(),
                          salt.data(),
                          crypto_pwhash_OPSLIMIT_INTERACTIVE,
                          crypto_pwhash_MEMLIMIT_INTERACTIVE,
                          crypto_pwhash_ALG_DEFAULT) != 0) {
                throw sodium_exception{"crypto_pwhash", "out of memory"};
        }

        return key;
}
