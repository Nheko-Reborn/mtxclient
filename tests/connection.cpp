#include <gtest/gtest.h>
#include <iostream>

#include "mtx/responses.hpp"
#include "mtxclient/http/client.hpp"
#include "mtxclient/http/errors.hpp"

#include "test_helpers.hpp"

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

TEST(Basic, Shutdown)
{
        std::shared_ptr<Client> client = std::make_shared<Client>("localhost");

        client->login("carl", "secret", [client](const mtx::responses::Login &, RequestErr err) {
                check_error(err);
        });

        while (client->access_token().empty())
                sleep();

        std::chrono::steady_clock::time_point begin = std::chrono::steady_clock::now();

        SyncOpts opts;
        opts.timeout = 40'000; // milliseconds
        client->sync(opts, [client, &opts](const mtx::responses::Sync &res, RequestErr err) {
                check_error(err);

                opts.since = res.next_batch;
                client->sync(opts, [](const mtx::responses::Sync &, RequestErr) {});
        });

        std::this_thread::sleep_for(std::chrono::seconds(1));

        // Force terminate all active connections.
        client->shutdown();
        client->close();

        std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();

        auto diff = std::chrono::duration_cast<std::chrono::seconds>(end - begin).count();
        ASSERT_TRUE(diff < 5);
}
