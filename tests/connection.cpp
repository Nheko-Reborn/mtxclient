#include <gtest/gtest.h>
#include <iostream>

#include "mtx/responses.hpp"
#include "mtxclient/http/client.hpp"
#include "mtxclient/http/errors.hpp"

using namespace mtx::http;
using namespace mtx::client;

using namespace std;

TEST(Basic, Connection)
{
        auto alice = std::make_shared<Client>("localhost", 8448);
        auto bob   = std::make_shared<Client>("localhost", 443);

        alice->versions(
          [](const mtx::responses::Versions &, RequestErr err) { ASSERT_FALSE(err); });
        bob->versions([](const mtx::responses::Versions &, RequestErr err) { ASSERT_FALSE(err); });

        bob->close();
        alice->close();
}

TEST(Basic, Failure)
{
        auto alice = std::make_shared<Client>("not-resolvable-example-domain.wrong");
        alice->versions([](const mtx::responses::Versions &, RequestErr err) { ASSERT_TRUE(err); });
        alice->close();
}
