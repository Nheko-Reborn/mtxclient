#include <atomic>

#include <boost/algorithm/string.hpp>
#include <boost/variant.hpp>

#include <gtest/gtest.h>

#include <nlohmann/json.hpp>

#include "mtx/events/collections.hpp"
#include "mtx/events/encrypted.hpp"
#include "mtx/requests.hpp"
#include "mtx/responses.hpp"
#include "mtxclient/http/client.hpp"

#include "test_helpers.hpp"

using namespace mtx::client;
using namespace mtx::http;
using namespace mtx::identifiers;
using namespace mtx::events::collections;
using namespace mtx::requests;

using namespace std;

TEST(ClientAPI, Register)
{
        auto user = std::make_shared<Client>("localhost");

        user->registration("alice", "secret", [](const mtx::responses::Register &, RequestErr err) {
                ASSERT_TRUE(err);
                EXPECT_EQ(mtx::errors::to_string(err->matrix_error.errcode), "M_USER_IN_USE");
        });

        auto username = utils::random_token(10, false);

        // Synapse converts the username to lowercase.
        boost::algorithm::to_lower(username);

        user->registration(
          username, "secret", [user, username](const mtx::responses::Register &, RequestErr err) {
                  if (!err || err->matrix_error.unauthorized.flows.size() == 0)
                          return;

                  EXPECT_EQ(err->matrix_error.unauthorized.flows.size(), 1);
                  EXPECT_EQ(err->matrix_error.unauthorized.flows[0].stages[0], "m.login.dummy");

                  user->registration(
                    username,
                    "secret",
                    {err->matrix_error.unauthorized.session, mtx::user_interactive::auth::Dummy{}},
                    [username](const mtx::responses::Register &res, RequestErr err) {
                            const auto user_id = "@" + username + ":localhost";

                            check_error(err);
                            EXPECT_EQ(res.user_id.to_string(), user_id);
                    });
          });

        user->close();
}

TEST(ClientAPI, LoginSuccess)
{
        std::shared_ptr<Client> mtx_client = std::make_shared<Client>("localhost");

        mtx_client->login("alice", "secret", [](const mtx::responses::Login &res, RequestErr err) {
                check_error(err);
                validate_login("@alice:localhost", res);
        });

        mtx_client->login("bob", "secret", [](const mtx::responses::Login &res, RequestErr err) {
                check_error(err);
                validate_login("@bob:localhost", res);
        });

        mtx_client->login("carl", "secret", [](const mtx::responses::Login &res, RequestErr err) {
                check_error(err);
                validate_login("@carl:localhost", res);
        });

        mtx_client->close();
}

TEST(ClientAPI, LoginWrongPassword)
{
        std::shared_ptr<Client> mtx_client = std::make_shared<Client>("localhost");

        mtx_client->login(
          "alice", "wrong_password", [](const mtx::responses::Login &res, RequestErr err) {
                  ASSERT_TRUE(err);
                  EXPECT_EQ(mtx::errors::to_string(err->matrix_error.errcode), "M_FORBIDDEN");
                  EXPECT_EQ(err->status_code, boost::beast::http::status::forbidden);

                  EXPECT_EQ(res.user_id.to_string(), "");
                  EXPECT_EQ(res.device_id, "");
                  EXPECT_EQ(res.access_token, "");
          });

        mtx_client->close();
}

TEST(ClientAPI, LoginWrongUsername)
{
        std::shared_ptr<Client> mtx_client = std::make_shared<Client>("localhost");

        mtx_client->login("john", "secret", [](const mtx::responses::Login &res, RequestErr err) {
                ASSERT_TRUE(err);
                EXPECT_EQ(mtx::errors::to_string(err->matrix_error.errcode), "M_FORBIDDEN");
                EXPECT_EQ(err->status_code, boost::beast::http::status::forbidden);

                EXPECT_EQ(res.user_id.to_string(), "");
                EXPECT_EQ(res.device_id, "");
                EXPECT_EQ(res.access_token, "");
        });

        mtx_client->close();
}

TEST(ClientAPI, LoginFlows)
{
        std::shared_ptr<Client> mtx_client = std::make_shared<Client>("localhost");

        mtx_client->get_login([](const mtx::responses::LoginFlows &res, RequestErr err) {
                ASSERT_FALSE(err);

                EXPECT_EQ(res.flows[0].type, mtx::user_interactive::auth_types::password);
        });

        mtx_client->close();
}

TEST(ClientAPI, SSORedirect)
{
        std::shared_ptr<Client> mtx_client = std::make_shared<Client>("localhost");
        EXPECT_EQ(mtx_client->login_sso_redirect("http://aaa:555/sso"),
                  "https://localhost:443/_matrix/client/r0/login/sso/"
                  "redirect?redirectUrl=http%3A%2F%2Faaa%3A555%2Fsso");
        mtx_client->close();
}

TEST(ClientAPI, DISABLED_EmptyUserAvatar)
{
        auto alice = std::make_shared<Client>("localhost");

        alice->login("alice", "secret", [alice](const mtx::responses::Login &res, RequestErr err) {
                ASSERT_FALSE(err);

                auto const alice_id = res.user_id;
                validate_login(alice_id.to_string(), res);

                alice->set_avatar_url("", [alice, alice_id](RequestErr err) {
                        ASSERT_FALSE(err);

                        auto done = false;

                        alice->get_profile(
                          alice_id.to_string(),
                          [&done](const mtx::responses::Profile &res, RequestErr err) {
                                  ASSERT_FALSE(err);
                                  ASSERT_TRUE(res.avatar_url.size() == 0);
                                  done = true;
                          });

                        while (!done)
                                sleep();

                        alice->get_avatar_url(
                          alice_id.to_string(),
                          [](const mtx::responses::AvatarUrl &res, RequestErr err) {
                                  ASSERT_FALSE(err);
                                  ASSERT_TRUE(res.avatar_url.size() == 0);
                          });
                });
        });

        alice->close();
}

TEST(ClientAPI, DISABLED_RealUserAvatar)
{
        auto alice = std::make_shared<Client>("localhost");

        alice->login("alice", "secret", [alice](const mtx::responses::Login &res, RequestErr err) {
                ASSERT_FALSE(err);

                auto const alice_id   = res.user_id;
                auto const avatar_url = "mxc://matrix.org/wefh34uihSDRGhw34";

                validate_login(alice_id.to_string(), res);

                alice->set_avatar_url(avatar_url, [alice, alice_id, avatar_url](RequestErr err) {
                        ASSERT_FALSE(err);

                        auto done = false;

                        alice->get_profile(
                          alice_id.to_string(),
                          [avatar_url, &done](const mtx::responses::Profile &res, RequestErr err) {
                                  ASSERT_FALSE(err);
                                  ASSERT_TRUE(res.avatar_url == avatar_url);
                                  done = true;
                          });

                        while (!done)
                                sleep();

                        alice->get_avatar_url(
                          alice_id.to_string(),
                          [avatar_url](const mtx::responses::AvatarUrl &res, RequestErr err) {
                                  ASSERT_FALSE(err);
                                  ASSERT_TRUE(res.avatar_url == avatar_url);
                          });
                });
        });

        alice->close();
}

TEST(ClientAPI, ChangeDisplayName)
{
        std::shared_ptr<Client> mtx_client = std::make_shared<Client>("localhost");

        mtx_client->login(
          "alice", "secret", [mtx_client](const mtx::responses::Login &, RequestErr err) {
                  check_error(err);

                  // Change the display name to Arthur Dent and verify its success through the lack
                  // of an error
                  mtx_client->set_displayname("Arthur Dent",
                                              [](RequestErr err) { check_error(err); });
          });

        mtx_client->close();
}

TEST(ClientAPI, EmptyDisplayName)
{
        std::shared_ptr<Client> mtx_client = std::make_shared<Client>("localhost");

        mtx_client->login(
          "alice", "secret", [mtx_client](const mtx::responses::Login &, RequestErr err) {
                  check_error(err);

                  // Change the display name to an empty string and verify its success through the
                  // lack of an error
                  mtx_client->set_displayname("", [](RequestErr err) { check_error(err); });
          });

        mtx_client->close();
}

TEST(ClientAPI, CreateRoom)
{
        std::shared_ptr<Client> mtx_client = std::make_shared<Client>("localhost");

        mtx_client->login(
          "alice", "secret", [mtx_client](const mtx::responses::Login &, RequestErr err) {
                  check_error(err);
          });

        while (mtx_client->access_token().empty())
                sleep();

        mtx::requests::CreateRoom req;
        req.name  = "Name";
        req.topic = "Topic";
        mtx_client->create_room(req, [](const mtx::responses::CreateRoom &res, RequestErr err) {
                check_error(err);
                ASSERT_TRUE(res.room_id.localpart().size() > 10);
                EXPECT_EQ(res.room_id.hostname(), "localhost");
        });

        mtx_client->close();
}

TEST(ClientAPI, TagRoom)
{
        std::shared_ptr<Client> mtx_client = std::make_shared<Client>("localhost");

        mtx_client->login(
          "alice", "secret", [mtx_client](const mtx::responses::Login &, RequestErr err) {
                  check_error(err);
          });

        while (mtx_client->access_token().empty())
                sleep();

        mtx::requests::CreateRoom req;
        req.name  = "Name";
        req.topic = "Topic";
        mtx_client->create_room(
          req, [mtx_client](const mtx::responses::CreateRoom &res, RequestErr err) {
                  auto room_id = res.room_id;
                  check_error(err); 

                  mtx_client->put_tag(
                    room_id.to_string(), "u.Test", {0.5}, [mtx_client, room_id](RequestErr err) {
                            check_error(err);

                            mtx_client->get_tags(
                              room_id.to_string(),
                              [mtx_client, room_id](mtx::events::account_data::Tags tags,
                                                    RequestErr err) {
                                      check_error(err);

                                      EXPECT_EQ(tags.tags.at("u.Test").order, 0.5);

                                      mtx_client->delete_tag(
                                        room_id.to_string(),
                                        "u.Test",
                                        [mtx_client, room_id](RequestErr err) {
                                                check_error(err);

                                                mtx_client->get_tags(
                                                  room_id.to_string(),
                                                  [mtx_client,
                                                   room_id](mtx::events::account_data::Tags tags,
                                                            RequestErr err) {
                                                          check_error(err);
                                                          EXPECT_EQ(tags.tags.count("u.Test"), 0);
                                                  });
                                        });
                              });
                    });
          });

        mtx_client->close();
}

TEST(ClientAPI, LogoutSuccess)
{
        std::shared_ptr<Client> mtx_client = std::make_shared<Client>("localhost");
        std::string token;

        // Login and prove that login was successful by creating a room
        mtx_client->login(
          "alice", "secret", [&token](const mtx::responses::Login &res, RequestErr err) {
                  check_error(err);
                  token = res.access_token;
          });

        while (token.empty())
                sleep();

        mtx_client->set_access_token(token);
        mtx::requests::CreateRoom req;
        req.name  = "Test1";
        req.topic = "Topic1";
        mtx_client->create_room(
          req, [](const mtx::responses::CreateRoom &, RequestErr err) { check_error(err); });

        // Logout and prove that logout was successful and deleted the access_token_ for the client
        mtx_client->logout([mtx_client, &token](const mtx::responses::Logout &, RequestErr err) {
                check_error(err);
                token.clear();
        });

        while (token.size())
                sleep();

        // Verify that sending requests with this mtx_client fails after logout
        mtx::requests::CreateRoom failReq;
        failReq.name  = "42";
        failReq.topic = "LifeUniverseEverything";
        mtx_client->create_room(failReq, [](const mtx::responses::CreateRoom &, RequestErr err) {
                ASSERT_TRUE(err);
                EXPECT_EQ(mtx::errors::to_string(err->matrix_error.errcode), "M_UNRECOGNIZED");
        });

        mtx_client->close();
}

TEST(ClientAPI, LogoutInvalidatesTokenOnServer)
{
        std::shared_ptr<Client> mtx_client = std::make_shared<Client>("localhost");
        std::string token;

        // Login and prove that login was successful by creating a room
        mtx_client->login(
          "alice", "secret", [&token](const mtx::responses::Login &res, RequestErr err) {
                  check_error(err);
                  token = res.access_token;
          });

        while (token.empty())
                sleep();

        mtx_client->set_access_token(token);
        mtx::requests::CreateRoom req;
        req.name  = "Test1";
        req.topic = "Topic1";
        mtx_client->create_room(
          req, [](const mtx::responses::CreateRoom &, RequestErr err) { check_error(err); });

        // Logout and prove that logout was successful by verifying the old access_token_ is no
        // longer valid
        mtx_client->logout([mtx_client, &token](const mtx::responses::Logout &, RequestErr err) {
                check_error(err);
                mtx_client->set_access_token(token);
                token.clear();
        });

        while (token.size())
                sleep();

        // Verify that creating a room with the old access_token_ no longer succeeds after logout
        mtx::requests::CreateRoom failReq;
        failReq.name  = "42";
        failReq.topic = "LifeUniverseEverything";
        mtx_client->create_room(failReq, [](const mtx::responses::CreateRoom &, RequestErr err) {
                ASSERT_TRUE(err);
                EXPECT_EQ(mtx::errors::to_string(err->matrix_error.errcode), "M_UNRECOGNIZED");
        });

        mtx_client->close();
}

TEST(ClientAPI, CreateRoomInvites)
{
        auto alice = std::make_shared<Client>("localhost");
        auto bob   = std::make_shared<Client>("localhost");
        auto carl  = std::make_shared<Client>("localhost");

        alice->login("alice", "secret", [alice](const mtx::responses::Login &, RequestErr err) {
                check_error(err);
        });

        bob->login("bob", "secret", [bob](const mtx::responses::Login &, RequestErr err) {
                check_error(err);
        });

        carl->login("carl", "secret", [carl](const mtx::responses::Login &, RequestErr err) {
                check_error(err);
        });

        while (alice->access_token().empty() || bob->access_token().empty() ||
               carl->access_token().empty())
                sleep();

        mtx::requests::CreateRoom req;
        req.name   = "Name";
        req.topic  = "Topic";
        req.invite = {"@bob:localhost", "@carl:localhost"};
        alice->create_room(req, [bob, carl](const mtx::responses::CreateRoom &res, RequestErr err) {
                check_error(err);
                auto room_id = res.room_id.to_string();

                bob->join_room(room_id, [](const mtx::responses::RoomId &, RequestErr err) {
                        check_error(err);
                });

                carl->join_room(room_id, [](const mtx::responses::RoomId &, RequestErr err) {
                        check_error(err);
                });
        });

        alice->close();
        bob->close();
        carl->close();
}

TEST(ClientAPI, JoinRoom)
{
        auto alice = std::make_shared<Client>("localhost");
        auto bob   = std::make_shared<Client>("localhost");

        alice->login("alice", "secret", [alice](const mtx::responses::Login &, RequestErr err) {
                check_error(err);
        });

        bob->login("bob", "secret", [bob](const mtx::responses::Login &, RequestErr err) {
                check_error(err);
        });

        while (alice->access_token().empty() || bob->access_token().empty())
                sleep();

        // Creating a random room alias.
        // TODO: add a type for room aliases.
        const auto alias = utils::random_token(20, false);

        mtx::requests::CreateRoom req;
        req.name            = "Name";
        req.topic           = "Topic";
        req.invite          = {"@bob:localhost"};
        req.room_alias_name = alias;
        alice->create_room(
          req, [bob, alias](const mtx::responses::CreateRoom &res, RequestErr err) {
                  check_error(err);
                  auto room_id = res.room_id.to_string();

                  bob->join_room(room_id, [](const mtx::responses::RoomId &, RequestErr err) {
                          check_error(err);
                  });

                  using namespace mtx::identifiers;
                  bob->join_room("!random_room_id:localhost",
                                 [](const mtx::responses::RoomId &, RequestErr err) {
                                         ASSERT_TRUE(err);
                                         EXPECT_EQ(
                                           mtx::errors::to_string(err->matrix_error.errcode),
                                           "M_UNKNOWN");
                                 });

                  // Join the room using an alias.
                  bob->join_room(
                    "#" + alias + ":localhost",
                    [](const mtx::responses::RoomId &, RequestErr err) { check_error(err); });
          });

        alice->close();
        bob->close();
}

TEST(ClientAPI, LeaveRoom)
{
        auto alice = std::make_shared<Client>("localhost");
        auto bob   = std::make_shared<Client>("localhost");

        alice->login("alice", "secret", [alice](const mtx::responses::Login &, RequestErr err) {
                check_error(err);
        });

        bob->login("bob", "secret", [bob](const mtx::responses::Login &, RequestErr err) {
                check_error(err);
        });

        while (alice->access_token().empty() || bob->access_token().empty())
                sleep();

        mtx::requests::CreateRoom req;
        req.name   = "Name";
        req.topic  = "Topic";
        req.invite = {"@bob:localhost"};
        alice->create_room(req, [bob](const mtx::responses::CreateRoom &res, RequestErr err) {
                check_error(err);
                auto room_id = res.room_id;

                bob->join_room(res.room_id.to_string(),
                               [room_id, bob](const mtx::responses::RoomId &, RequestErr err) {
                                       check_error(err);

                                       bob->leave_room(room_id.to_string(),
                                                       [](mtx::responses::Empty, RequestErr err) {
                                                               check_error(err);
                                                       });
                               });
        });

        // Trying to leave a non-existent room should fail.
        bob->leave_room("!random_room_id:localhost", [](mtx::responses::Empty, RequestErr err) {
                ASSERT_TRUE(err);
                EXPECT_EQ(mtx::errors::to_string(err->matrix_error.errcode), "M_UNKNOWN");
                EXPECT_EQ(err->matrix_error.error, "Not a known room");
        });

        alice->close();
        bob->close();
}

TEST(ClientAPI, InviteRoom)
{
        auto alice = std::make_shared<Client>("localhost");
        auto bob   = std::make_shared<Client>("localhost");

        alice->login("alice", "secret", [alice](const mtx::responses::Login &, RequestErr err) {
                check_error(err);
        });

        bob->login("bob", "secret", [bob](const mtx::responses::Login &, RequestErr err) {
                check_error(err);
        });

        while (alice->access_token().empty() || bob->access_token().empty())
                sleep();

        mtx::requests::CreateRoom req;
        req.name   = "Name";
        req.topic  = "Topic";
        req.invite = {};
        alice->create_room(
          req, [alice, bob](const mtx::responses::CreateRoom &res, RequestErr err) {
                  check_error(err);
                  auto room_id = res.room_id.to_string();

                  alice->invite_user(room_id,
                                     "@bob:localhost",
                                     [room_id, bob](const mtx::responses::Empty &, RequestErr err) {
                                             check_error(err);

                                             bob->join_room(
                                               room_id,
                                               [](const mtx::responses::RoomId &, RequestErr err) {
                                                       check_error(err);
                                               });
                                     });
          });

        alice->close();
        bob->close();
}

TEST(ClientAPI, KickRoom)
{
        auto alice = std::make_shared<Client>("localhost");
        auto bob   = std::make_shared<Client>("localhost");

        alice->login("alice", "secret", [alice](const mtx::responses::Login &, RequestErr err) {
                check_error(err);
        });

        bob->login("bob", "secret", [bob](const mtx::responses::Login &, RequestErr err) {
                check_error(err);
        });

        while (alice->access_token().empty() || bob->access_token().empty())
                sleep();

        mtx::requests::CreateRoom req;
        req.name   = "Name";
        req.topic  = "Topic";
        req.invite = {};
        alice->create_room(
          req, [alice, bob](const mtx::responses::CreateRoom &res, RequestErr err) {
                  check_error(err);
                  auto room_id = res.room_id.to_string();

                  alice->invite_user(
                    room_id,
                    "@bob:localhost",
                    [room_id, alice, bob](const mtx::responses::Empty &, RequestErr err) {
                            check_error(err);

                            bob->join_room(
                              room_id,
                              [alice, room_id](const mtx::responses::RoomId &, RequestErr err) {
                                      check_error(err);

                                      alice->kick_user(room_id,
                                                       "@bob:localhost",
                                                       [](const mtx::responses::Empty &,
                                                          RequestErr err) { check_error(err); });
                              });
                    });
          });

        alice->close();
        bob->close();
}

TEST(ClientAPI, BanRoom)
{
        auto alice = std::make_shared<Client>("localhost");
        auto bob   = std::make_shared<Client>("localhost");

        alice->login("alice", "secret", [alice](const mtx::responses::Login &, RequestErr err) {
                check_error(err);
        });

        bob->login("bob", "secret", [bob](const mtx::responses::Login &, RequestErr err) {
                check_error(err);
        });

        while (alice->access_token().empty() || bob->access_token().empty())
                sleep();

        mtx::requests::CreateRoom req;
        req.name   = "Name";
        req.topic  = "Topic";
        req.invite = {};
        alice->create_room(
          req, [alice, bob](const mtx::responses::CreateRoom &res, RequestErr err) {
                  check_error(err);
                  auto room_id = res.room_id.to_string();

                  alice->invite_user(
                    room_id,
                    "@bob:localhost",
                    [room_id, alice, bob](const mtx::responses::Empty &, RequestErr err) {
                            check_error(err);

                            bob->join_room(
                              room_id,
                              [alice, room_id](const mtx::responses::RoomId &, RequestErr err) {
                                      check_error(err);

                                      alice->ban_user(
                                        room_id,
                                        "@bob:localhost",
                                        [alice, room_id](const mtx::responses::Empty &,
                                                         RequestErr err) {
                                                check_error(err);
                                                alice->unban_user(
                                                  room_id,
                                                  "@bob:localhost",
                                                  [](const mtx::responses::Empty &,
                                                     RequestErr err) { check_error(err); },
                                                  "You not bad anymore!");
                                        },
                                        "You bad!");
                              });
                    });
          });

        alice->close();
        bob->close();
}

TEST(ClientAPI, InvalidInvite)
{
        auto alice = std::make_shared<Client>("localhost");
        auto bob   = std::make_shared<Client>("localhost");

        alice->login("alice", "secret", [alice](const mtx::responses::Login &, RequestErr err) {
                check_error(err);
        });

        bob->login("bob", "secret", [bob](const mtx::responses::Login &, RequestErr err) {
                check_error(err);
        });

        while (alice->access_token().empty() || bob->access_token().empty())
                sleep();

        mtx::requests::CreateRoom req;
        req.name   = "Name";
        req.topic  = "Topic";
        req.invite = {};
        alice->create_room(
          req, [alice, bob](const mtx::responses::CreateRoom &res, RequestErr err) {
                  check_error(err);
                  auto room_id = res.room_id.to_string();

                  bob->invite_user(room_id,
                                   "@carl:localhost",
                                   [room_id, bob](const mtx::responses::Empty &, RequestErr err) {
                                           ASSERT_TRUE(err);
                                           EXPECT_EQ(
                                             mtx::errors::to_string(err->matrix_error.errcode),
                                             "M_FORBIDDEN");
                                   });
          });

        alice->close();
        bob->close();
}

TEST(ClientAPI, Sync)
{
        std::shared_ptr<Client> mtx_client = std::make_shared<Client>("localhost");

        mtx_client->login(
          "alice", "secret", [mtx_client](const mtx::responses::Login &, RequestErr err) {
                  check_error(err);
          });

        while (mtx_client->access_token().empty())
                sleep();

        mtx::requests::CreateRoom req;
        req.name  = "Name";
        req.topic = "Topic";
        mtx_client->create_room(
          req, [mtx_client](const mtx::responses::CreateRoom &, RequestErr err) {
                  check_error(err);

                  SyncOpts opts;
                  opts.timeout = 0;
                  mtx_client->sync(opts, [](const mtx::responses::Sync &res, RequestErr err) {
                          check_error(err);
                          ASSERT_TRUE(res.rooms.join.size() > 0);
                          ASSERT_TRUE(res.next_batch.size() > 0);
                  });
          });

        mtx_client->close();
}

TEST(ClientAPI, Versions)
{
        std::shared_ptr<Client> mtx_client = std::make_shared<Client>("localhost");

        mtx_client->versions([](const mtx::responses::Versions &res, RequestErr err) {
                check_error(err);

                EXPECT_EQ(res.versions.size(), 7);
                EXPECT_EQ(res.versions.at(0), "r0.0.1");
                EXPECT_EQ(res.versions.at(1), "r0.1.0");
                EXPECT_EQ(res.versions.at(2), "r0.2.0");
                EXPECT_EQ(res.versions.at(3), "r0.3.0");
                EXPECT_EQ(res.versions.at(4), "r0.4.0");
                EXPECT_EQ(res.versions.at(5), "r0.5.0");
                EXPECT_EQ(res.versions.at(6), "r0.6.0");
        });

        mtx_client->close();
}

TEST(ClientAPI, DISABLED_Typing)
{
        auto alice = std::make_shared<Client>("localhost");

        alice->login("alice", "secret", [](const mtx::responses::Login &, RequestErr err) {
                check_error(err);
        });

        while (alice->access_token().empty())
                sleep();

        mtx::requests::CreateRoom req;
        alice->create_room(req, [alice](const mtx::responses::CreateRoom &res, RequestErr err) {
                check_error(err);

                alice->start_typing(res.room_id.to_string(), 10000, [alice, res](RequestErr err) {
                        check_error(err);

                        const auto room_id = res.room_id.to_string();
                        atomic_bool can_continue(false);

                        SyncOpts opts;
                        opts.timeout = 0;
                        alice->sync(
                          opts,
                          [room_id, &can_continue](const mtx::responses::Sync &res,
                                                   RequestErr err) {
                                  check_error(err);

                                  can_continue = true;

                                  auto room = res.rooms.join.at(room_id);

                                  EXPECT_EQ(room.ephemeral.events.size(), 1);
                                  EXPECT_EQ(
                                    std::get<
                                      mtx::events::EphemeralEvent<mtx::events::ephemeral::Typing>>(
                                      room.ephemeral.events.front())
                                      .content.user_ids.front(),
                                    "@alice:localhost");
                          });

                        while (!can_continue)
                                sleep();

                        alice->stop_typing(
                          res.room_id.to_string(), [alice, room_id](RequestErr err) {
                                  check_error(err);

                                  SyncOpts opts;
                                  opts.timeout = 0;
                                  alice->sync(
                                    opts,
                                    [room_id](const mtx::responses::Sync &res, RequestErr err) {
                                            check_error(err);
                                            auto room = res.rooms.join.at(room_id);
                                            EXPECT_EQ(room.ephemeral.events.size(), 1);
                                            EXPECT_EQ(std::get<mtx::events::EphemeralEvent<
                                                        mtx::events::ephemeral::Typing>>(
                                                        room.ephemeral.events.front())
                                                        .content.user_ids.size(),
                                                      0);
                                    });
                          });
                });
        });

        alice->close();
}

TEST(ClientAPI, Presence)
{
        auto alice = std::make_shared<Client>("localhost");

        alice->login("alice", "secret", [](const mtx::responses::Login &, RequestErr err) {
                check_error(err);
        });

        while (alice->access_token().empty())
                sleep();

        alice->put_presence_status(
          mtx::presence::unavailable, "Is this thing on?", [alice](RequestErr err) {
                  check_error(err);

                  alice->presence_status(
                    alice->user_id().to_string(),
                    [alice](const mtx::events::presence::Presence &presence, RequestErr err) {
                            check_error(err);

                            EXPECT_EQ(presence.presence, mtx::presence::unavailable);
                            EXPECT_EQ(presence.status_msg, "Is this thing on?");

                            alice->put_presence_status(
                              mtx::presence::offline, std::nullopt, [alice](RequestErr err) {
                                      check_error(err);

                                      alice->presence_status(
                                        alice->user_id().to_string(),
                                        [alice](const mtx::events::presence::Presence &presence,
                                                RequestErr err) {
                                                check_error(err);

                                                EXPECT_EQ(presence.presence,
                                                          mtx::presence::offline);
                                                EXPECT_TRUE(presence.status_msg.empty());
                                        });
                              });
                    });
          });

        alice->close();
}

TEST(ClientAPI, PresenceOverSync)
{
        auto alice = std::make_shared<Client>("localhost");
        auto bob   = std::make_shared<Client>("localhost");

        alice->login("alice", "secret", [](const mtx::responses::Login &, RequestErr err) {
                check_error(err);
        });
        bob->login(
          "bob", "secret", [](const mtx::responses::Login &, RequestErr err) { check_error(err); });

        while (alice->access_token().empty() && bob->access_token().empty())
                sleep();

        mtx::requests::CreateRoom req;
        req.invite = {"@bob:localhost"};
        alice->create_room(
          req, [alice, bob](const mtx::responses::CreateRoom &res, RequestErr err) {
                  check_error(err);
                  auto room_id = res.room_id.to_string();

                  bob->join_room(
                    room_id, [alice, bob, room_id](const mtx::responses::RoomId &, RequestErr err) {
                            check_error(err);
                            alice->put_presence_status(
                              mtx::presence::unavailable,
                              "Is this thing on?",
                              [alice, bob](RequestErr err) {
                                      check_error(err);

                                      SyncOpts opts;
                                      opts.timeout      = 10;
                                      opts.set_presence = mtx::presence::online;
                                      alice->sync(
                                        opts,
                                        [bob, opts](const mtx::responses::Sync &, RequestErr err) {
                                                check_error(err);

                                                bob->sync(
                                                  opts,
                                                  [bob](const mtx::responses::Sync &s,
                                                        RequestErr err) {
                                                          check_error(err);

                                                          ASSERT_GE(s.presence.size(), 1);

                                                          bool found = false;
                                                          for (const auto &p : s.presence) {
                                                                  if (p.sender ==
                                                                      "@alice:localhost") {
                                                                          found = true;
                                                                          EXPECT_EQ(
                                                                            p.content.presence,
                                                                            mtx::presence::online);
                                                                          EXPECT_EQ(
                                                                            p.content.status_msg,
                                                                            "Is this thing "
                                                                            "on?");
                                                                  }
                                                                  EXPECT_TRUE(found);
                                                          }
                                                  });
                                        });
                              });
                    });
          });

        alice->close();
}

TEST(ClientAPI, DISABLED_SendMessages)
{
        auto alice = std::make_shared<Client>("localhost");
        auto bob   = std::make_shared<Client>("localhost");

        alice->login("alice", "secret", [alice](const mtx::responses::Login &, RequestErr err) {
                check_error(err);
        });

        bob->login("bob", "secret", [bob](const mtx::responses::Login &, RequestErr err) {
                check_error(err);
        });

        while (alice->access_token().empty() || bob->access_token().empty())
                sleep();

        mtx::requests::CreateRoom req;
        req.invite = {"@bob:localhost"};
        alice->create_room(
          req, [alice, bob](const mtx::responses::CreateRoom &res, RequestErr err) {
                  check_error(err);
                  auto room_id = res.room_id.to_string();

                  bob->join_room(
                    room_id, [alice, bob, room_id](const mtx::responses::RoomId &, RequestErr err) {
                            check_error(err);

                            // Flag to indicate when those messages would be ready to be read by
                            // alice.
                            vector<string> event_ids;

                            mtx::events::msg::Text text;
                            text.body = "hello alice!";

                            bob->send_room_message<mtx::events::msg::Text>(
                              room_id,
                              text,
                              [&event_ids](const mtx::responses::EventId &res, RequestErr err) {
                                      event_ids.push_back(res.event_id.to_string());
                                      check_error(err);
                              });

                            mtx::events::msg::Emote emote;
                            emote.body = "*bob tests";

                            bob->send_room_message<mtx::events::msg::Emote>(
                              room_id,
                              emote,
                              [&event_ids](const mtx::responses::EventId &res, RequestErr err) {
                                      event_ids.push_back(res.event_id.to_string());
                                      check_error(err);
                              });

                            while (event_ids.size() != 2)
                                    sleep();

                            SyncOpts opts;
                            opts.timeout = 0;
                            alice->sync(opts,
                                        [room_id, event_ids](const mtx::responses::Sync &res,
                                                             RequestErr err) {
                                                check_error(err);

                                                auto ids = get_event_ids<TimelineEvents>(
                                                  res.rooms.join.at(room_id).timeline.events);

                                                // The sent event ids should be visible in the
                                                // timeline.
                                                for (const auto &event_id : event_ids)
                                                        ASSERT_TRUE(std::find(ids.begin(),
                                                                              ids.end(),
                                                                              event_id) !=
                                                                    std::end(ids));
                                        });
                    });
          });

        alice->close();
        bob->close();
}

TEST(ClientAPI, RedactEvent)
{
        auto alice = std::make_shared<Client>("localhost");
        alice->login("alice", "secret", check_login);

        while (alice->access_token().empty())
                sleep();

        mtx::requests::CreateRoom req;
        alice->create_room(req, [alice](const mtx::responses::CreateRoom &res, RequestErr err) {
                check_error(err);
                auto room_id = res.room_id.to_string();

                mtx::events::msg::Text text;
                text.body = "hello alice!";

                alice->send_room_message<mtx::events::msg::Text>(
                  room_id,
                  text,
                  [room_id, alice](const mtx::responses::EventId &res, RequestErr err) {
                          check_error(err);

                          alice->redact_event(
                            room_id,
                            res.event_id.to_string(),
                            [](const mtx::responses::EventId &res, RequestErr err) {
                                    check_error(err);
                                    ASSERT_FALSE(res.event_id.to_string().empty());
                            });
                  });
        });

        alice->close();
}

TEST(ClientAPI, DISABLED_SendStateEvents)
{
        auto alice = std::make_shared<Client>("localhost");
        auto bob   = std::make_shared<Client>("localhost");

        alice->login("alice", "secret", [alice](const mtx::responses::Login &, RequestErr err) {
                check_error(err);
        });

        bob->login("bob", "secret", [bob](const mtx::responses::Login &, RequestErr err) {
                check_error(err);
        });

        while (alice->access_token().empty() || bob->access_token().empty())
                sleep();

        mtx::requests::CreateRoom req;
        req.invite = {"@bob:localhost"};
        alice->create_room(
          req, [alice, bob](const mtx::responses::CreateRoom &res, RequestErr err) {
                  check_error(err);
                  auto room_id = res.room_id;

                  // Flag to indicate when those messages would be ready to be read by
                  // alice.
                  vector<string> event_ids;

                  mtx::events::state::Name event;
                  event.name = "Bob's room";

                  bob->send_state_event<mtx::events::state::Name>(
                    room_id.to_string(),
                    event,
                    [](const mtx::responses::EventId &, RequestErr err) {
                            ASSERT_TRUE(err);
                            ASSERT_EQ("M_FORBIDDEN",
                                      mtx::errors::to_string(err->matrix_error.errcode));
                    });

                  mtx::events::state::Name name_event;
                  name_event.name = "Alice's room";
                  alice->send_state_event<mtx::events::state::Name>(
                    room_id.to_string(),
                    name_event,
                    [&event_ids](const mtx::responses::EventId &res, RequestErr err) {
                            check_error(err);
                            event_ids.push_back(res.event_id.to_string());
                    });

                  mtx::events::state::Avatar avatar;
                  avatar.url = "mxc://localhost/random";
                  alice->send_state_event<mtx::events::state::Avatar>(
                    room_id.to_string(),
                    avatar,
                    [&event_ids](const mtx::responses::EventId &res, RequestErr err) {
                            check_error(err);
                            event_ids.push_back(res.event_id.to_string());
                    });

                  while (event_ids.size() != 2)
                          sleep();

                  SyncOpts opts;
                  opts.timeout = 0;
                  alice->sync(
                    opts, [room_id, event_ids](const mtx::responses::Sync &res, RequestErr err) {
                            check_error(err);

                            auto ids = get_event_ids<TimelineEvents>(
                              res.rooms.join.at(room_id.to_string()).timeline.events);

                            // The sent event ids should be visible in the timeline.
                            for (const auto &event_id : event_ids)
                                    ASSERT_TRUE(std::find(ids.begin(), ids.end(), event_id) !=
                                                std::end(ids));
                    });
          });

        alice->close();
        bob->close();
}

TEST(ClientAPI, Pagination)
{
        auto alice = std::make_shared<Client>("localhost");

        alice->login("alice", "secret", [alice](const mtx::responses::Login &, RequestErr err) {
                check_error(err);
        });

        while (alice->access_token().empty())
                sleep();

        mtx::requests::CreateRoom req;
        alice->create_room(req, [alice](const mtx::responses::CreateRoom &res, RequestErr err) {
                check_error(err);
                auto room_id = res.room_id;

                MessagesOpts opts;
                opts.room_id = res.room_id.to_string();

                alice->messages(
                  opts, [room_id, alice](const mtx::responses::Messages &res, RequestErr err) {
                          check_error(err);

                          ASSERT_TRUE(res.chunk.size() > 5);
                          ASSERT_NE(res.start, res.end);

                          MessagesOpts opts;
                          opts.from    = res.end;
                          opts.room_id = room_id.to_string();
                          alice->messages(opts,
                                          [](const mtx::responses::Messages &res, RequestErr err) {
                                                  check_error(err);

                                                  // We reached the start of the timeline.
                                                  EXPECT_EQ(res.start, res.end);
                                                  EXPECT_EQ(res.chunk.size(), 0);
                                          });
                  });
        });

        alice->close();
}

TEST(ClientAPI, UploadFilter)
{
        auto alice = std::make_shared<Client>("localhost");

        alice->login("alice", "secret", [alice](const mtx::responses::Login &, RequestErr err) {
                check_error(err);
        });

        while (alice->access_token().empty())
                sleep();

        nlohmann::json j = {
          {"room", {{"include_leave", true}, {"account_data", {{"not_types", {"*"}}}}}},
          {"account_data", {{"not_types", {"*"}}}},
          {"presence", {{"not_types", {"*"}}}}};

        alice->upload_filter(j, [](const mtx::responses::FilterId &res, RequestErr err) {
                check_error(err);
                ASSERT_TRUE(res.filter_id.size() > 0);
        });

        alice->close();
}

TEST(ClientAPI, DISABLED_ReadMarkers)
{
        auto alice = std::make_shared<Client>("localhost");

        alice->login("alice", "secret", [alice](const mtx::responses::Login &, RequestErr err) {
                check_error(err);
        });

        while (alice->access_token().empty())
                sleep();

        mtx::requests::CreateRoom req;
        alice->create_room(req, [alice](const mtx::responses::CreateRoom &res, RequestErr err) {
                check_error(err);

                string event_id;

                mtx::events::msg::Text text;
                text.body = "hello alice!";

                const auto room_id = res.room_id;

                alice->send_room_message<mtx::events::msg::Text>(
                  room_id.to_string(),
                  text,
                  [alice, &event_id, room_id](const mtx::responses::EventId &res, RequestErr err) {
                          check_error(err);

                          alice->read_event(room_id.to_string(),
                                            res.event_id.to_string(),
                                            [&event_id, res](RequestErr err) {
                                                    check_error(err);
                                                    event_id = res.event_id.to_string();
                                            });
                  });

                while (event_id.size() == 0)
                        sleep();

                SyncOpts opts;
                opts.timeout = 0;
                alice->sync(
                  opts, [room_id, event_id](const mtx::responses::Sync &res, RequestErr err) {
                          check_error(err);

                          auto receipts = res.rooms.join.at(room_id.to_string()).ephemeral.events;
                          EXPECT_EQ(receipts.size(), 1);

                          auto users =
                            std::get<mtx::events::EphemeralEvent<mtx::events::ephemeral::Receipt>>(
                              receipts.front())
                              .content.receipts[event_id];
                          EXPECT_EQ(users.users.size(), 1);
                          ASSERT_TRUE(users.users["@alice:localhost"].ts > 0);
                  });
        });

        alice->close();
}

TEST(ClientAPI, SendToDevice)
{
        auto alice = std::make_shared<Client>("localhost");
        auto bob   = std::make_shared<Client>("localhost");

        alice->login("alice", "secret", &check_login);
        bob->login("bob", "secret", &check_login);

        while (alice->access_token().empty() || bob->access_token().empty())
                sleep();

        json body{{"messages",
                   {{bob->user_id().to_string(),
                     {{bob->device_id(),
                       {
                         {"action", "request"},
                         {"body",
                          {{"sender_key", "test"},
                           {"algorithm", "test_algo"},
                           {"room_id", "test_room_id"},
                           {"session_id", "test_session_id"}}},
                         {"request_id", "test_request_id"},
                         {"requesting_device_id", "test_req_id"},
                       }}}}}}};

        alice->send_to_device("m.room_key_request", body, [bob](RequestErr err) {
                check_error(err);

                SyncOpts opts;
                opts.timeout = 0;
                bob->sync(opts, [](const mtx::responses::Sync &res, RequestErr err) {
                        check_error(err);

                        EXPECT_EQ(res.to_device.events.size(), 1);

                        auto event = std::get<mtx::events::DeviceEvent<msgs::KeyRequest>>(
                          res.to_device.events[0]);
                        EXPECT_EQ(event.content.action, mtx::events::msg::RequestAction::Request);
                        EXPECT_EQ(event.content.sender_key, "test");
                        EXPECT_EQ(event.content.algorithm, "test_algo");
                        EXPECT_EQ(event.content.room_id, "test_room_id");
                        EXPECT_EQ(event.content.session_id, "test_session_id");
                        EXPECT_EQ(event.content.request_id, "test_request_id");
                        EXPECT_EQ(event.content.requesting_device_id, "test_req_id");
                        EXPECT_EQ(event.type, mtx::events::EventType::RoomKeyRequest);
                        EXPECT_EQ(event.sender, "@alice:localhost");
                });
        });

        alice->close();
        bob->close();
}

TEST(ClientAPI, NewSendToDevice)
{
        auto alice = std::make_shared<Client>("localhost");
        auto bob   = std::make_shared<Client>("localhost");
        auto carl  = std::make_shared<Client>("localhost");

        alice->login("alice", "secret", &check_login);
        bob->login("bob", "secret", &check_login);
        carl->login("carl", "secret", &check_login);

        while (alice->access_token().empty() || bob->access_token().empty() ||
               carl->access_token().empty())
                sleep();

        ToDeviceMessages<msgs::KeyRequest> body1;
        ToDeviceMessages<msgs::KeyRequest> body2;

        msgs::KeyRequest request1;

        request1.action               = mtx::events::msg::RequestAction::Request;
        request1.sender_key           = "test";
        request1.algorithm            = "m.megolm.v1.aes-sha2";
        request1.room_id              = "test_room_id";
        request1.session_id           = "test_session_id";
        request1.request_id           = "test_request_id";
        request1.requesting_device_id = "test_req_id";

        body1[bob->user_id()][bob->device_id()] = request1;

        msgs::KeyRequest request2;

        request2.action               = mtx::events::msg::RequestAction::Cancellation;
        request2.request_id           = "test_request_id_1";
        request2.requesting_device_id = "test_req_id_1";

        body2[bob->user_id()][bob->device_id()] = request2;

        carl->send_to_device(
          "m.room.key_request", body1, [bob](RequestErr err) { check_error(err); });

        alice->send_to_device("m.room_key_request", body2, [bob](RequestErr err) {
                check_error(err);

                SyncOpts opts;
                opts.timeout = 0;
                bob->sync(opts, [](const mtx::responses::Sync &res, RequestErr err) {
                        check_error(err);

                        EXPECT_EQ(res.to_device.events.size(), 2);
                        auto event = std::get<mtx::events::DeviceEvent<msgs::KeyRequest>>(
                          res.to_device.events[0]);
                });
        });

        alice->close();
        bob->close();
        carl->close();
}

TEST(ClientAPI, RetrieveSingleEvent)
{
        auto bob = std::make_shared<Client>("localhost");
        bob->login("bob", "secret", check_login);

        while (bob->access_token().empty())
                sleep();

        mtx::requests::CreateRoom req;
        bob->create_room(req, [bob](const mtx::responses::CreateRoom &res, RequestErr err) {
                check_error(err);
                auto room_id = res.room_id.to_string();

                mtx::events::msg::Text text;
                text.body = "Hello Alice!";

                bob->send_room_message<mtx::events::msg::Text>(
                  room_id,
                  text,
                  [room_id, bob](const mtx::responses::EventId &res, RequestErr err) {
                          check_error(err);

                          bob->get_event(
                            room_id,
                            res.event_id.to_string(),
                            [event_id = res.event_id.to_string()](
                              const mtx::events::collections::TimelineEvents &res, RequestErr err) {
                                    check_error(err);
                                    ASSERT_TRUE(
                                      std::holds_alternative<
                                        mtx::events::RoomEvent<mtx::events::msg::Text>>(res));
                                    auto e =
                                      std::get<mtx::events::RoomEvent<mtx::events::msg::Text>>(res);
                                    EXPECT_EQ(e.content.body, "Hello Alice!");
                                    EXPECT_EQ(e.sender, "@bob:localhost");
                                    EXPECT_EQ(e.event_id, event_id);
                            });

                          bob->get_event(
                            room_id,
                            "$random_event:localhost",
                            [event_id = res.event_id.to_string()](
                              const mtx::events::collections::TimelineEvents &, RequestErr err) {
                                    ASSERT_TRUE(err);
                                    EXPECT_EQ(static_cast<int>(err->status_code), 404);
                            });
                  });
        });

        bob->close();
}

TEST(Groups, DISABLED_Rooms)
{
        auto alice = std::make_shared<Client>("localhost");
        alice->login("alice", "secret", check_login);

        WAIT_UNTIL(!alice->access_token().empty())

        std::atomic<int> rooms_created(0);
        std::vector<std::string> rooms_ids;

        auto create_room_cb = [&rooms_created, &rooms_ids](const mtx::responses::CreateRoom &res,
                                                           RequestErr err) {
                check_error(err);
                rooms_created += 1;
                rooms_ids.push_back(res.room_id.to_string());
        };

        mtx::requests::CreateRoom req;
        alice->create_room(req, create_room_cb);
        alice->create_room(req, create_room_cb);

        WAIT_UNTIL(rooms_created == 2)

        auto random_group_id = to_string(random_number());

        alice->create_group(
          random_group_id,
          [alice, rooms_ids, random_group_id](const mtx::responses::GroupId &res, RequestErr err) {
                  check_error(err);

                  std::atomic<int> rooms_added(0);

                  alice->add_room_to_group(
                    rooms_ids.at(0), res.group_id, [&rooms_added](RequestErr err) {
                            check_error(err);
                            rooms_added += 1;
                    });

                  alice->add_room_to_group(
                    rooms_ids.at(1), res.group_id, [&rooms_added](RequestErr err) {
                            check_error(err);
                            rooms_added += 1;
                    });

                  WAIT_UNTIL(rooms_added == 2)

                  alice->joined_groups(
                    [random_group_id](const mtx::responses::JoinedGroups &res, RequestErr err) {
                            check_error(err);

                            ASSERT_GE(res.groups.size(), 1);

                            for (const auto &g : res.groups) {
                                    if (g == std::string("+" + random_group_id + ":localhost"))
                                            return;
                            }

                            FAIL();
                    });

                  alice->group_rooms("+" + random_group_id + ":localhost",
                                     [](const nlohmann::json &res, RequestErr err) {
                                             check_error(err);
                                             EXPECT_GE(res.at("chunk").size(), 2);
                                     });
          });

        alice->close();
}

TEST(Groups, DISABLED_Profiles)
{
        auto alice = std::make_shared<Client>("localhost");
        alice->login("alice", "secret", check_login);

        WAIT_UNTIL(!alice->access_token().empty())

        auto random_group_id = to_string(random_number());
        alice->create_group(
          random_group_id,
          [alice, random_group_id](const mtx::responses::GroupId &res, RequestErr err) {
                  check_error(err);

                  EXPECT_GE(res.group_id.size(), random_group_id.size());

                  json profile;
                  profile["name"] = "Name";
                  alice->set_group_profile(
                    "+" + random_group_id + ":localhost",
                    profile,
                    [alice, random_group_id](const nlohmann::json &, RequestErr err) {
                            check_error(err);

                            alice->group_profile(
                              "+" + random_group_id + ":localhost",
                              [](const mtx::responses::GroupProfile &res, RequestErr err) {
                                      check_error(err);
                                      EXPECT_EQ(res.name, "Name");
                                      EXPECT_EQ(res.avatar_url, "");
                              });
                    });
          });

        alice->close();
}

TEST(ClientAPI, PublicRooms)  
{
        // Setup : Create a new (public) room with some settings, add a user to the room
        auto alice = std::make_shared<Client>("localhost");
        auto bob   = std::make_shared<Client>("localhost");

        alice->login("alice", "secret", [alice](const mtx::responses::Login &, RequestErr err) {
                check_error(err);
        });

        bob->login("bob", "secret", [bob](const mtx::responses::Login &, RequestErr err) {
                check_error(err);
        });

        while (alice->access_token().empty() || bob->access_token().empty())
                sleep();

        std::cout << "Alice AT: " << alice->access_token() << "\n";
        std::cout << "Bob AT: " << bob->access_token() << "\n";

        mtx::requests::CreateRoom req;
        req.name = "Public Room";
        req.topic = "Test";
        req.visibility = Visibility::Private;
        req.invite = {"@bob:localhost"};
        // req.room_alias_name = "foo";

        alice->create_room(
                req, [alice, bob](const mtx::responses::CreateRoom &res, RequestErr err){
                check_error(err);
                auto room_id = res.room_id;

                // TEST 1: endpoints to set and get the visibility of the room we just created
                mtx::requests::RoomVisibility r;
                r.visibility = mtx::requests::Visibility::Public;

                std::cout << "HERE\n";

                        alice->put_room_visibility(room_id.to_string(), r, [alice, bob, room_id](RequestErr err){
                                check_error(err);

                                std::cout << "PLS WORK!\n";

                                alice->get_room_visibility
                                        ("", [alice, room_id](const mtx::responses::RoomVisibility &, RequestErr err) {
                                                ASSERT_TRUE(err);
                                                EXPECT_EQ(mtx::errors::to_string(err->matrix_error.errcode),
                                                "M_NOT_FOUND");
                                });

                                std::atomic<bool> done = false;

                                alice->get_room_visibility
                                        (room_id.to_string(), [alice, bob, room_id, &done](const mtx::responses::RoomVisibility &res, RequestErr err){
                                        
                                        check_error(err);
                                        EXPECT_EQ(visibilityToString(res.visibility), "public");
                                        done = true;
                                
                                        // TEST 2: endpoints to add and list the public rooms on the server 
                                        mtx::requests::PublicRooms room_req;
                                        room_req.limit = 1;
                                        room_req.include_all_networks = true;
                                        json j = room_req;
                                        std::cout << j.dump(4) << "\n";

                                        while (!done) {
                                                sleep();
                                        }

                                        alice->post_public_rooms
                                        (room_req, [alice, bob, room_id, room_req](const mtx::responses::PublicRooms &, RequestErr err) {
                                                check_error(err);

                                                std::cout << "POST req\n";

                                                // alice->get_public_rooms
                                                // ([alice, bob, room_id](const mtx::responses::PublicRooms &res, RequestErr err) {
                                                //         check_error(err);
                                                //         std::cout << res.chunk.size() << std::endl;
                                                //         EXPECT_EQ(res.chunk[0].name, "Public Room");
                                                //         EXPECT_EQ(res.chunk[0].topic, "Test");
                                                //         EXPECT_EQ(res.chunk[0].num_joined_members, 2);
                                                // }, "", 1);
                                        });         
                                });
                        });

                // bob->join_room(room_id.to_string(), [alice, bob, room_id](const mtx::responses::RoomId &, RequestErr err) {
                //         check_error(err);

                //         // TEST 1: endpoints to set and get the visibility of the room we just created
                //         mtx::requests::RoomVisibility r;
                //         r.visibility = mtx::requests::Visibility::Private;

                //         std::cout << "HERE\n";

                //         alice->put_room_visibility(room_id.to_string(), r, [alice, bob, room_id](RequestErr err){
                //                 check_error(err);

                //                 std::cout << "PLS WORK!\n";

                //                 alice->get_room_visibility
                //                         ("", [alice, room_id](const mtx::responses::RoomVisibility &, RequestErr err) {
                //                                 check_error(err);
                //                                 ASSERT_TRUE(err);
                //                                 EXPECT_EQ(mtx::errors::to_string(err->matrix_error.errcode),
                //                                 "M_NOT_FOUND");
                //                 });

                //                 // alice->get_room_visibility
                //                 //         (room_id, [alice](const mtx::responses::RoomVisibility &res, RequestErr err){
                                        
                //                 //         check_error(err);
                //                 //         EXPECT_EQ(visibilityToString(res.visibility), "public");
                                
                //                 //         // TEST 2: endpoints to add and list the public rooms on the server 
                //                 //         mtx::requests::PublicRooms room_req;
                //                 //         room_req.limit = 1;
                //                 //         room_req.include_all_networks = true;
                //                 //         json j = room_req;
                //                 //         std::cout << j.dump(4) << "\n";

                //                 //         alice->post_public_rooms
                //                 //         (room_req, [alice](const mtx::responses::PublicRooms &, RequestErr err) {
                //                 //                 check_error(err);

                //                 //                 alice->get_public_rooms
                //                 //                 ([alice](const mtx::responses::PublicRooms &res, RequestErr err) {
                //                 //                         check_error(err);
                //                 //                         std::cout << res.chunk.size() << std::endl;
                //                 //                         EXPECT_EQ(res.chunk[0].name, "Public Room");
                //                 //                         EXPECT_EQ(res.chunk[0].topic, "Test");
                //                 //                         EXPECT_EQ(res.chunk[0].num_joined_members, 2);
                //                 //                 }, "", 1);
                //                 //         });         
                //                 // });
                //         });
                // });
        });
        alice->close();
        bob->close();  
}
