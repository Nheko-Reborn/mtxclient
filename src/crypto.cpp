#include <sodium.h>

#include "crypto.hpp"

using json = nlohmann::json;
using namespace mtx::client::crypto;

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
