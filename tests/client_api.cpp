#include <chrono>
#include <thread>

#include <gtest/gtest.h>

#include "client.hpp"
#include "mtx/requests.hpp"
#include "mtx/responses.hpp"

using namespace mtx::client;
using namespace mtx::identifiers;

using namespace std;

using ErrType = std::experimental::optional<errors::ClientError>;

void
validate_login(const std::string &user, const mtx::responses::Login &res)
{
        EXPECT_EQ(res.user_id.toString(), user);
        EXPECT_EQ(res.home_server, "localhost");
        ASSERT_TRUE(res.access_token.size() > 100);
        ASSERT_TRUE(res.device_id.size() > 5);
}

TEST(ClientAPI, LoginSuccess)
{
        std::shared_ptr<Client> mtx_client = std::make_shared<Client>("localhost");

        mtx_client->login("alice", "secret", [](const mtx::responses::Login &res, ErrType err) {
                ASSERT_FALSE(err);
                validate_login("@alice:localhost", res);
        });

        mtx_client->login("bob", "secret", [](const mtx::responses::Login &res, ErrType err) {
                ASSERT_FALSE(err);
                validate_login("@bob:localhost", res);
        });

        mtx_client->login("carl", "secret", [](const mtx::responses::Login &res, ErrType err) {
                ASSERT_FALSE(err);
                validate_login("@carl:localhost", res);
        });

        mtx_client->close();
}

TEST(ClientAPI, LoginWrongPassword)
{
        std::shared_ptr<Client> mtx_client = std::make_shared<Client>("localhost");

        mtx_client->login(
          "alice", "wrong_password", [](const mtx::responses::Login &res, ErrType err) {
                  ASSERT_TRUE(err);
                  EXPECT_EQ(mtx::errors::to_string(err->matrix_error.errcode), "M_FORBIDDEN");
                  EXPECT_EQ(err->status_code, boost::beast::http::status::forbidden);

                  EXPECT_EQ(res.user_id.toString(), "");
                  EXPECT_EQ(res.device_id, "");
                  EXPECT_EQ(res.home_server, "");
                  EXPECT_EQ(res.access_token, "");
          });

        mtx_client->close();
}

TEST(ClientAPI, LoginWrongUsername)
{
        std::shared_ptr<Client> mtx_client = std::make_shared<Client>("localhost");

        mtx_client->login("john", "secret", [](const mtx::responses::Login &res, ErrType err) {
                ASSERT_TRUE(err);
                EXPECT_EQ(mtx::errors::to_string(err->matrix_error.errcode), "M_FORBIDDEN");
                EXPECT_EQ(err->status_code, boost::beast::http::status::forbidden);

                EXPECT_EQ(res.user_id.toString(), "");
                EXPECT_EQ(res.device_id, "");
                EXPECT_EQ(res.home_server, "");
                EXPECT_EQ(res.access_token, "");
        });

        mtx_client->close();
}

TEST(ClientAPI, CreateRoom)
{
        std::shared_ptr<Client> mtx_client = std::make_shared<Client>("localhost");

        mtx_client->login(
          "alice", "secret", [mtx_client](const mtx::responses::Login &res, ErrType err) {
                  ASSERT_FALSE(err);
                  validate_login("@alice:localhost", res);
          });

        // Waiting for the previous request to complete.
        std::this_thread::sleep_for(std::chrono::seconds(3));

        mtx::requests::CreateRoom req;
        req.name  = "Name";
        req.topic = "Topic";
        mtx_client->create_room(req, [](const mtx::responses::CreateRoom &res, ErrType err) {
                ASSERT_FALSE(err);
                ASSERT_TRUE(res.room_id.localpart().size() > 10);
                EXPECT_EQ(res.room_id.hostname(), "localhost");
        });

        mtx_client->close();
}

TEST(ClientAPI, LogoutSuccess)
{
        std::shared_ptr<Client> mtx_client = std::make_shared<Client>("localhost");
        std::string token;

        // Login and prove that login was successful by creating a room
        mtx_client->login(
          "alice", "secret", [&token](const mtx::responses::Login &res, ErrType err) {
                  ASSERT_FALSE(err);
                  validate_login("@alice:localhost", res);
                  token = res.access_token;
          });
        while (token.empty()) {
                // Block while we are logging in
                std::this_thread::sleep_for(std::chrono::milliseconds(300));
        }
        mtx_client->set_access_token(token);
        mtx::requests::CreateRoom req;
        req.name  = "Test1";
        req.topic = "Topic1";
        mtx_client->create_room(req, [](const mtx::responses::CreateRoom &res, ErrType err) {
                boost::ignore_unused(res);
                ASSERT_FALSE(err);
        });

        // Logout and prove that logout was successful and deleted the access_token_ for the client
        mtx_client->logout([mtx_client, &token](const mtx::responses::Logout &res, ErrType err) {
                boost::ignore_unused(res);
                ASSERT_FALSE(err);
                token.clear();
        });
        while (token.size()) {
                // Block while we are logging out
                std::this_thread::sleep_for(std::chrono::milliseconds(300));
        }
        // Verify that sending requests with this mtx_client fails after logout
        mtx::requests::CreateRoom failReq;
        failReq.name  = "42";
        failReq.topic = "LifeUniverseEverything";
        mtx_client->create_room(failReq, [](const mtx::responses::CreateRoom &res, ErrType err) {
                boost::ignore_unused(res);
                ASSERT_TRUE(err);
                EXPECT_EQ(mtx::errors::to_string(err->matrix_error.errcode), "M_MISSING_TOKEN");
                EXPECT_EQ(err->status_code, boost::beast::http::status::forbidden);

        });

        mtx_client->close();
}

TEST(ClientAPI, LogoutInvalidatesTokenOnServer)
{
        std::shared_ptr<Client> mtx_client = std::make_shared<Client>("localhost");
        std::string token;

        // Login and prove that login was successful by creating a room
        mtx_client->login(
          "alice", "secret", [&token](const mtx::responses::Login &res, ErrType err) {
                  ASSERT_FALSE(err);
                  validate_login("@alice:localhost", res);
                  token = res.access_token;
          });
        while (token.empty()) {
                // Block while we are logging in
                std::this_thread::sleep_for(std::chrono::milliseconds(300));
        }
        mtx_client->set_access_token(token);
        mtx::requests::CreateRoom req;
        req.name  = "Test1";
        req.topic = "Topic1";
        mtx_client->create_room(req, [](const mtx::responses::CreateRoom &res, ErrType err) {
                boost::ignore_unused(res);
                ASSERT_FALSE(err);
        });

        // Logout and prove that logout was successful by verifying the old access_token_ is no
        // longer valid
        mtx_client->logout([mtx_client, &token](const mtx::responses::Logout &res, ErrType err) {
                boost::ignore_unused(res);
                ASSERT_FALSE(err);
                mtx_client->set_access_token(token);
                token.clear();
        });
        while (token.size()) {
                // Block while we are logging out
                std::this_thread::sleep_for(std::chrono::milliseconds(300));
        }
        // Verify that creating a room with the old access_token_ no longer succeeds after logout
        mtx::requests::CreateRoom failReq;
        failReq.name  = "42";
        failReq.topic = "LifeUniverseEverything";
        mtx_client->create_room(failReq, [](const mtx::responses::CreateRoom &res, ErrType err) {
                boost::ignore_unused(res);
                ASSERT_TRUE(err);
                EXPECT_EQ(mtx::errors::to_string(err->matrix_error.errcode), "M_UNKNOWN_TOKEN");
                EXPECT_EQ(err->status_code, boost::beast::http::status::forbidden);
        });

        mtx_client->close();
}

TEST(ClientAPI, CreateRoomInvites)
{
        auto alice = std::make_shared<Client>("localhost");
        auto bob   = std::make_shared<Client>("localhost");
        auto carl  = std::make_shared<Client>("localhost");

        alice->login("alice", "secret", [alice](const mtx::responses::Login &res, ErrType err) {
                boost::ignore_unused(res);
                ASSERT_FALSE(err);
        });

        bob->login("bob", "secret", [bob](const mtx::responses::Login &res, ErrType err) {
                boost::ignore_unused(res);
                ASSERT_FALSE(err);
        });

        carl->login("carl", "secret", [carl](const mtx::responses::Login &res, ErrType err) {
                boost::ignore_unused(res);
                ASSERT_FALSE(err);
        });

        // Waiting for the previous requests to complete.
        std::this_thread::sleep_for(std::chrono::seconds(3));

        mtx::requests::CreateRoom req;
        req.name   = "Name";
        req.topic  = "Topic";
        req.invite = {"@bob:localhost", "@carl:localhost"};
        alice->create_room(req, [bob, carl](const mtx::responses::CreateRoom &res, ErrType err) {
                ASSERT_FALSE(err);
                auto room_id = res.room_id;

                bob->join_room(res.room_id,
                               [](const nlohmann::json &, ErrType err) { ASSERT_FALSE(err); });

                carl->join_room(res.room_id,
                                [](const nlohmann::json &, ErrType err) { ASSERT_FALSE(err); });
        });

        alice->close();
        bob->close();
        carl->close();
}

TEST(ClientAPI, JoinRoom)
{
        auto alice = std::make_shared<Client>("localhost");
        auto bob   = std::make_shared<Client>("localhost");

        alice->login("alice", "secret", [alice](const mtx::responses::Login &res, ErrType err) {
                boost::ignore_unused(res);
                ASSERT_FALSE(err);
        });

        bob->login("bob", "secret", [bob](const mtx::responses::Login &res, ErrType err) {
                boost::ignore_unused(res);
                ASSERT_FALSE(err);
        });

        // Waiting for the previous requests to complete.
        std::this_thread::sleep_for(std::chrono::seconds(3));

        // Creating a random room alias.
        // TODO: add a type for room aliases.
        const auto alias = utils::random_token(20, false);

        mtx::requests::CreateRoom req;
        req.name            = "Name";
        req.topic           = "Topic";
        req.invite          = {"@bob:localhost"};
        req.room_alias_name = alias;
        alice->create_room(req, [bob, alias](const mtx::responses::CreateRoom &res, ErrType err) {
                ASSERT_FALSE(err);
                auto room_id = res.room_id;

                bob->join_room(res.room_id,
                               [](const nlohmann::json &, ErrType err) { ASSERT_FALSE(err); });

                using namespace mtx::identifiers;
                bob->join_room(parse<Room>("!random_room_id:localhost"),
                               [](const nlohmann::json &, ErrType err) {
                                       ASSERT_TRUE(err);
                                       EXPECT_EQ(mtx::errors::to_string(err->matrix_error.errcode),
                                                 "M_UNRECOGNIZED");
                               });

                // Join the room using an alias.
                bob->join_room("#" + alias + ":localhost",
                               [](const nlohmann::json &, ErrType err) { ASSERT_FALSE(err); });
        });

        alice->close();
        bob->close();
}

TEST(ClientAPI, LeaveRoom)
{
        auto alice = std::make_shared<Client>("localhost");
        auto bob   = std::make_shared<Client>("localhost");

        alice->login("alice", "secret", [alice](const mtx::responses::Login &res, ErrType err) {
                boost::ignore_unused(res);
                ASSERT_FALSE(err);
        });

        bob->login("bob", "secret", [bob](const mtx::responses::Login &res, ErrType err) {
                boost::ignore_unused(res);
                ASSERT_FALSE(err);
        });

        // Waiting for the previous requests to complete.
        std::this_thread::sleep_for(std::chrono::seconds(3));

        mtx::requests::CreateRoom req;
        req.name   = "Name";
        req.topic  = "Topic";
        req.invite = {"@bob:localhost"};
        alice->create_room(req, [bob](const mtx::responses::CreateRoom &res, ErrType err) {
                ASSERT_FALSE(err);
                auto room_id = res.room_id;

                bob->join_room(res.room_id, [room_id, bob](const nlohmann::json &, ErrType err) {
                        ASSERT_FALSE(err);

                        bob->leave_room(
                          room_id, [](const nlohmann::json &, ErrType err) { ASSERT_FALSE(err); });
                });
        });

        // Trying to leave a non-existent room should fail.
        bob->leave_room(
          parse<Room>("!random_room_id:localhost"), [](const nlohmann::json &, ErrType err) {
                  ASSERT_TRUE(err);
                  EXPECT_EQ(mtx::errors::to_string(err->matrix_error.errcode), "M_UNRECOGNIZED");
                  EXPECT_EQ(err->matrix_error.error, "Not a known room");
          });

        alice->close();
        bob->close();
}

TEST(ClientAPI, Sync)
{
        std::shared_ptr<Client> mtx_client = std::make_shared<Client>("localhost");

        mtx_client->login(
          "alice", "secret", [mtx_client](const mtx::responses::Login &res, ErrType err) {
                  boost::ignore_unused(res);
                  ASSERT_FALSE(err);
          });

        // Waiting for the previous request to complete.
        std::this_thread::sleep_for(std::chrono::seconds(2));

        mtx::requests::CreateRoom req;
        req.name  = "Name";
        req.topic = "Topic";
        mtx_client->create_room(
          req, [](const mtx::responses::CreateRoom &, ErrType err) { ASSERT_FALSE(err); });

        // Waiting for the previous request to complete.
        std::this_thread::sleep_for(std::chrono::seconds(2));

        mtx_client->sync("", "", false, 0, [](const mtx::responses::Sync &res, ErrType err) {
                ASSERT_FALSE(err);
                ASSERT_TRUE(res.rooms.join.size() > 0);
                ASSERT_TRUE(res.next_batch.size() > 0);
        });

        mtx_client->close();
}
