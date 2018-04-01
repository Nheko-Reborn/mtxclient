#include <iostream>
#include <sodium.h>

#include "crypto.hpp"
#include <olm/base64.hh>

using json = nlohmann::json;
using namespace mtx::client::crypto;

constexpr std::size_t SIGNATURE_SIZE = 64;

std::unique_ptr<uint8_t[]>
mtx::client::crypto::create_buffer(std::size_t nbytes)
{
        auto buf = std::make_unique<uint8_t[]>(nbytes);
        randombytes_buf(buf.get(), nbytes);

        return buf;
}

std::shared_ptr<olm::Account>
mtx::client::crypto::olm_new_account()
{
        auto olm_account = std::make_shared<olm::Account>();

        const auto nbytes = olm_account->new_account_random_length();
        auto buf          = create_buffer(nbytes);

        int result = olm_account->new_account(buf.get(), nbytes);

        if (result == -1)
                throw olm_exception("olm_new_account", olm_account->last_error);

        return olm_account;
}

json
mtx::client::crypto::identity_keys(std::shared_ptr<olm::Account> account)
{
        const auto nbytes = account->get_identity_json_length();
        auto buf          = create_buffer(nbytes);

        int result = account->get_identity_json(buf.get(), nbytes);

        if (result == -1)
                throw olm_exception("identity_keys", account->last_error);

        std::string data(buf.get(), buf.get() + nbytes);

        return json::parse(data);
}

std::size_t
mtx::client::crypto::generate_one_time_keys(std::shared_ptr<olm::Account> account,
                                            std::size_t number_of_keys)
{
        const auto nbytes = account->generate_one_time_keys_random_length(number_of_keys);

        auto buf = create_buffer(nbytes);
        return account->generate_one_time_keys(number_of_keys, buf.get(), nbytes);
}

json
mtx::client::crypto::one_time_keys(std::shared_ptr<olm::Account> account)
{
        const auto nbytes = account->get_one_time_keys_json_length();
        auto buf          = create_buffer(nbytes);

        int result = account->get_one_time_keys_json(buf.get(), nbytes);

        if (result == -1)
                throw olm_exception("one_time_keys", account->last_error);

        std::string data(buf.get(), buf.get() + nbytes);

        return json::parse(data);
}

std::string
mtx::client::crypto::sign_one_time_key(std::shared_ptr<olm::Account> account,
                                       const std::string &key)
{
        json j{{"key", key}};
        auto str_json = j.dump();

        auto signature_buf = sign_message(account, j.dump());

        return encode_base64(signature_buf.get(), SIGNATURE_SIZE);
}

std::unique_ptr<uint8_t[]>
mtx::client::crypto::sign_message(std::shared_ptr<olm::Account> account, const std::string &msg)
{
        // Message buffer
        auto buf           = str_to_buffer(msg);
        std::size_t nbytes = msg.size();

        // Signature buffer
        auto signature_buf = create_buffer(SIGNATURE_SIZE);
        account->sign(buf.get(), nbytes, signature_buf.get(), SIGNATURE_SIZE);

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

std::unique_ptr<uint8_t[]>
mtx::client::crypto::str_to_buffer(const std::string &data)
{
        auto str_pointer  = reinterpret_cast<const uint8_t *>(&data[0]);
        const auto nbytes = data.size();

        auto buf = create_buffer(nbytes);
        memcpy(buf.get(), str_pointer, nbytes);

        return buf;
}

std::unique_ptr<uint8_t[]>
mtx::client::crypto::decode_base64(const std::string &data)
{
        const auto nbytes       = data.size();
        const int output_nbytes = olm::decode_base64_length(nbytes);

        if (output_nbytes == -1)
                throw std::runtime_error("invalid base64 input length");

        auto output_buf = create_buffer(output_nbytes);
        auto input_buf  = str_to_buffer(data);

        olm::decode_base64(input_buf.get(), nbytes, output_buf.get());

        return output_buf;
}

std::string
mtx::client::crypto::encode_base64(const uint8_t *data, std::size_t len)
{
        const int output_nbytes = olm::encode_base64_length(len);

        if (output_nbytes == -1)
                throw std::runtime_error("invalid base64 input length");

        auto output_buf = create_buffer(output_nbytes);
        olm::encode_base64(data, output_nbytes, output_buf.get());

        return std::string(output_buf.get(), output_buf.get() + output_nbytes);
}

std::unique_ptr<uint8_t[]>
mtx::client::crypto::json_to_buffer(const nlohmann::json &obj)
{
        return str_to_buffer(obj.dump());
}
