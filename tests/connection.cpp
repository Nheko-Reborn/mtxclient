#include <gtest/gtest.h>
#include <iostream>

#include "client.hpp"
#include "errors.hpp"
#include "mtx/responses.hpp"

using namespace mtx::client;

using namespace std;

using ErrType = std::experimental::optional<errors::ClientError>;

TEST(Basic, Connection)
{
        auto alice = std::make_shared<Client>("localhost", 8448);
        auto bob   = std::make_shared<Client>("localhost", 443);

        alice->versions([](const mtx::responses::Versions &, ErrType err) { ASSERT_FALSE(err); });
        bob->versions([](const mtx::responses::Versions &, ErrType err) { ASSERT_FALSE(err); });

        bob->close();
        alice->close();
}

TEST(Basic, Failure)
{
        auto alice = std::make_shared<Client>("not-resolvable-example-domain.wrong");
        alice->versions([](const mtx::responses::Versions &, ErrType err) { ASSERT_TRUE(err); });
        alice->close();
}
