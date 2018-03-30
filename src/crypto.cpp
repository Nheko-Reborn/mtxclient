#include "crypto.hpp"

using json = nlohmann::json;

std::shared_ptr<olm::Account::Account>
mtx::client::crypto::olm_new_account()
{
        auto olm_account    = std::make_shared<olm::Account::Account>();
        const auto buf_size = olm_account->new_account_random_length();
        auto account_buf    = std::make_unique<uint8_t[]>(buf_size);

        int result = olm_account->new_account(account_buf.get(), buf_size);

        if (result == -1)
                throw olm_exception("olm_new_account", olm_account->last_error);

        return olm_account;
}

json
mtx::client::crypto::identity_keys(std::shared_ptr<olm::Account::Account> account)
{
        const auto buf_size = account->get_identity_json_length();
        auto json_buf       = std::make_unique<uint8_t[]>(buf_size);

        int result = account->get_identity_json(json_buf.get(), buf_size);

        if (result == -1)
                throw olm_exception("identity_keys", account->last_error);

        std::string data(json_buf.get(), json_buf.get() + buf_size);

        return json::parse(data);
}

json
mtx::client::crypto::one_time_keys(std::shared_ptr<olm::Account::Account> account)
{
        const auto buf_size = account->get_one_time_keys_json_length();
        auto json_buf       = std::make_unique<uint8_t[]>(buf_size);

        int result = account->get_one_time_keys_json(json_buf.get(), buf_size);

        if (result == -1)
                throw olm_exception("one_time_keys", account->last_error);

        std::string data(json_buf.get(), json_buf.get() + buf_size);

        return json::parse(data);
}
