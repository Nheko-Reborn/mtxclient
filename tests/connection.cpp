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
    auto client = make_test_client();

    client->versions([](const mtx::responses::Versions &, RequestErr err) { check_error(err); });
    client->close();
}

TEST(Basic, ServerWithPort)
{
    std::string server = server_name();
    auto alice         = std::make_shared<Client>("example.org");
    alice->verify_certificates(false);
    alice->set_server(server + ":8008");

    EXPECT_EQ(alice->server(), server);
    EXPECT_EQ(alice->port(), 8008);

    alice->versions([](const mtx::responses::Versions &, RequestErr err) { check_error(err); });
    alice->close();
}

TEST(Basic, Failure)
{
    auto alice = std::make_shared<Client>("not-resolvable-example-domain.wrong");
    alice->verify_certificates(false);
    alice->versions([](const mtx::responses::Versions &, RequestErr err) { ASSERT_TRUE(err); });
    alice->close();
}

TEST(Basic, Shutdown)
{
    std::shared_ptr<Client> client = make_test_client();

    client->login("carl", "secret", [client](const mtx::responses::Login &, RequestErr err) {
        check_error(err);
    });

    while (client->access_token().empty())
        sleep();

    std::chrono::steady_clock::time_point begin = std::chrono::steady_clock::now();

    SyncOpts opts;
    opts.timeout = 1'000; // milliseconds
    client->sync(opts, [client, &opts](const mtx::responses::Sync &res, RequestErr err) {
        check_error(err);

        opts.since   = res.next_batch;
        opts.timeout = 60'000; // milliseconds
        client->sync(opts, [](const mtx::responses::Sync &, RequestErr) {});
    });

    std::this_thread::sleep_for(std::chrono::seconds(2));

    // Force terminate all active connections.
    client->shutdown();
    client->close();

    std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();

    auto diff = std::chrono::duration_cast<std::chrono::seconds>(end - begin).count();
    ASSERT_TRUE(diff < 5);
}

TEST(Basic, ExceptionInHandler)
{
    auto client = make_test_client();

    int counter = 0;
    client->versions([&counter](const mtx::responses::Versions &, RequestErr) {
        counter++;
        throw std::logic_error("This is just a drill.");
    });
    client->close();
    EXPECT_EQ(counter, 1);
}
