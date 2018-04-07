#pragma once

#include <gtest/gtest.h>
#include <iostream>
#include <string>
#include <thread>
#include <vector>

#include "client.hpp"

inline void
sleep()
{
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
}

inline void
check_error(mtx::client::RequestErr err)
{
        if (err) {
                std::cout << "matrix (error)  : " << err->matrix_error.error << "\n";
                std::cout << "matrix (errcode): "
                          << mtx::errors::to_string(err->matrix_error.errcode) << "\n";
                std::cout << "error_code      : " << err->error_code << "\n";
                std::cout << "status_code     : " << err->status_code << "\n";

                if (!err->parse_error.empty())
                        std::cout << "parse_error     : " << err->parse_error << "\n";
        }

        ASSERT_FALSE(err);
}

inline void
check_login(const mtx::responses::Login &, mtx::client::RequestErr err)
{
        check_error(err);
}

inline void
validate_login(const std::string &user, const mtx::responses::Login &res)
{
        EXPECT_EQ(res.user_id.to_string(), user);
        EXPECT_EQ(res.home_server, "localhost");
        ASSERT_TRUE(res.access_token.size() > 100);
        ASSERT_TRUE(res.device_id.size() > 5);
}

template<class Collection>
std::vector<std::string>
get_event_ids(const std::vector<Collection> &events)
{
        std::vector<std::string> ids;

        for (const auto &e : events)
                ids.push_back(mpark::visit([](auto msg) { return msg.event_id; }, e));

        return ids;
}
