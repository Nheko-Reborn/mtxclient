#include <iostream>
#include <sodium.h>

#include "crypto.hpp"
#include <olm/base64.hh>

using json = nlohmann::json;
using namespace mtx::client::crypto;

constexpr std::size_t SIGNATURE_SIZE = 64;

std::unique_ptr<BinaryBuf>
mtx::client::crypto::create_buffer(std::size_t nbytes)
{
        auto buf = std::make_unique<BinaryBuf>(nbytes);
        randombytes_buf(buf->data(), buf->size());

        return buf;
}

std::shared_ptr<olm::Account>
mtx::client::crypto::olm_new_account()
{
        auto olm_account = std::make_shared<olm::Account>();

        const auto nbytes = olm_account->new_account_random_length();
        auto buf          = create_buffer(nbytes);

        int result = olm_account->new_account(buf->data(), buf->size());

        if (result == -1)
                throw olm_exception("olm_new_account", olm_account->last_error);

        return olm_account;
}

IdentityKeys
mtx::client::crypto::identity_keys(std::shared_ptr<olm::Account> account)
{
        const auto nbytes = account->get_identity_json_length();
        auto buf          = create_buffer(nbytes);

        int result = account->get_identity_json(buf->data(), buf->size());

        if (result == -1)
                throw olm_exception("identity_keys", account->last_error);

        std::string data(buf->begin(), buf->end());
        IdentityKeys keys = json::parse(data);

        return keys;
}

std::string
mtx::client::crypto::sign_identity_keys(std::shared_ptr<olm::Account> account,
                                        const IdentityKeys &keys,
                                        const mtx::identifiers::User &user_id,
                                        const std::string &device_id)
{
        json body{{"algorithms", {"m.olm.v1.curve25519-aes-sha2", "m.megolm.v1.aes-sha2"}},
                  {"user_id", user_id.to_string()},
                  {"device_id", device_id},
                  {"keys",
                   {
                     {"curve25519:" + device_id, keys.curve25519},
                     {"ed25519:" + device_id, keys.ed25519},
                   }}};

        return encode_base64(sign_message(account, body.dump())->data(), SIGNATURE_SIZE);
}

std::size_t
mtx::client::crypto::generate_one_time_keys(std::shared_ptr<olm::Account> account,
                                            std::size_t number_of_keys)
{
        const auto nbytes = account->generate_one_time_keys_random_length(number_of_keys);

        auto buf = create_buffer(nbytes);
        return account->generate_one_time_keys(number_of_keys, buf->data(), buf->size());
}

json
mtx::client::crypto::one_time_keys(std::shared_ptr<olm::Account> account)
{
        const auto nbytes = account->get_one_time_keys_json_length();
        auto buf          = create_buffer(nbytes);

        int result = account->get_one_time_keys_json(buf->data(), buf->size());

        if (result == -1)
                throw olm_exception("one_time_keys", account->last_error);

        std::string data(buf->begin(), buf->end());

        return json::parse(data);
}

std::string
mtx::client::crypto::sign_one_time_key(std::shared_ptr<olm::Account> account,
                                       const std::string &key)
{
        json j{{"key", key}};
        auto str_json = j.dump();

        auto signature_buf = sign_message(account, j.dump());

        return encode_base64(signature_buf->data(), signature_buf->size());
}

std::map<std::string, json>
mtx::client::crypto::sign_one_time_keys(std::shared_ptr<olm::Account> account,
                                        const mtx::client::crypto::OneTimeKeys &keys,
                                        const mtx::identifiers::User &user_id,
                                        const std::string &device_id)
{
        // Sign & append the one time keys.
        std::map<std::string, json> signed_one_time_keys;
        for (const auto &elem : keys.curve25519) {
                auto sig = sign_one_time_key(account, elem.second);

                signed_one_time_keys["signed_curve25519:" + elem.first] =
                  signed_one_time_key_json(user_id, device_id, elem.second, sig);
        }

        return signed_one_time_keys;
}

std::unique_ptr<BinaryBuf>
mtx::client::crypto::sign_message(std::shared_ptr<olm::Account> account, const std::string &msg)
{
        // Message buffer
        auto buf = str_to_buffer(msg);

        // Signature buffer
        auto signature_buf = create_buffer(SIGNATURE_SIZE);
        account->sign(buf->data(), buf->size(), signature_buf->data(), signature_buf->size());

        return signature_buf;
}

json
mtx::client::crypto::signed_one_time_key_json(const mtx::identifiers::User &user_id,
                                              const std::string &device_id,
                                              const std::string &key,
                                              const std::string &signature)
{
        return json{{"key", key},
                    {"signatures", {{user_id.to_string(), {{"ed25519:" + device_id, signature}}}}}};
}

std::unique_ptr<BinaryBuf>
mtx::client::crypto::str_to_buffer(const std::string &data)
{
        auto str_pointer  = reinterpret_cast<const uint8_t *>(&data[0]);
        const auto nbytes = data.size();

        auto buf = create_buffer(nbytes);
        memcpy(buf->data(), str_pointer, buf->size());

        return buf;
}

std::unique_ptr<BinaryBuf>
mtx::client::crypto::decode_base64(const std::string &data)
{
        const auto nbytes       = data.size();
        const int output_nbytes = olm::decode_base64_length(nbytes);

        if (output_nbytes == -1)
                throw std::runtime_error("invalid base64 input length");

        auto output_buf = create_buffer(output_nbytes);
        auto input_buf  = str_to_buffer(data);

        olm::decode_base64(input_buf->data(), nbytes, output_buf->data());

        return output_buf;
}

std::string
mtx::client::crypto::encode_base64(const uint8_t *data, std::size_t len)
{
        const int output_nbytes = olm::encode_base64_length(len);

        if (output_nbytes == -1)
                throw std::runtime_error("invalid base64 input length");

        auto output_buf = create_buffer(output_nbytes);
        olm::encode_base64(data, len, output_buf->data());

        return std::string(output_buf->begin(), output_buf->end());
}

std::unique_ptr<BinaryBuf>
mtx::client::crypto::json_to_buffer(const nlohmann::json &obj)
{
        return str_to_buffer(obj.dump());
}
