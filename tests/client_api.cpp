#include <atomic>
#include <iostream>

#include <boost/algorithm/string.hpp>

#include <gtest/gtest.h>

#include "client.hpp"
#include "mtx/requests.hpp"
#include "mtx/responses.hpp"
#include "variant.hpp"

#include "test_helpers.hpp"

using namespace mtx::client;
using namespace mtx::identifiers;
using namespace mtx::events::collections;

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

        user->flow_register(
          username,
          "secret",
          [user, username](const mtx::responses::RegistrationFlows &res, RequestErr) {
                  if (res.flows.size() == 0)
                          return;

                  EXPECT_EQ(res.flows.size(), 2);
                  EXPECT_EQ(res.flows[0].stages[0], "m.login.dummy");
                  EXPECT_EQ(res.flows[1].stages[0], "m.login.email.identity");

                  user->flow_response(
                    username,
                    "secret",
                    res.session,
                    "m.login.dummy",
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
                  EXPECT_EQ(res.home_server, "");
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
                EXPECT_EQ(res.home_server, "");
                EXPECT_EQ(res.access_token, "");
        });

        mtx_client->close();
}

TEST(ClientAPI, EmptyUserAvatar)
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
                          alice_id, [&done](const mtx::responses::Profile &res, RequestErr err) {
                                  ASSERT_FALSE(err);
                                  ASSERT_TRUE(res.avatar_url.size() == 0);
                                  done = true;
                          });

                        while (!done)
                                sleep();

                        alice->get_avatar_url(
                          alice_id, [](const mtx::responses::AvatarUrl &res, RequestErr err) {
                                  ASSERT_FALSE(err);
                                  ASSERT_TRUE(res.avatar_url.size() == 0);
                          });
                });
        });

        alice->close();
}

TEST(ClientAPI, RealUserAvatar)
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
                          alice_id,
                          [avatar_url, &done](const mtx::responses::Profile &res, RequestErr err) {
                                  ASSERT_FALSE(err);
                                  ASSERT_TRUE(res.avatar_url == avatar_url);
                                  done = true;
                          });

                        while (!done)
                                sleep();

                        alice->get_avatar_url(
                          alice_id,
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
                auto room_id = res.room_id;

                bob->join_room(res.room_id,
                               [](const nlohmann::json &, RequestErr err) { check_error(err); });

                carl->join_room(res.room_id,
                                [](const nlohmann::json &, RequestErr err) { check_error(err); });
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
                  auto room_id = res.room_id;

                  bob->join_room(res.room_id,
                                 [](const nlohmann::json &, RequestErr err) { check_error(err); });

                  using namespace mtx::identifiers;
                  bob->join_room(parse<Room>("!random_room_id:localhost"),
                                 [](const nlohmann::json &, RequestErr err) {
                                         ASSERT_TRUE(err);
                                         EXPECT_EQ(
                                           mtx::errors::to_string(err->matrix_error.errcode),
                                           "M_UNRECOGNIZED");
                                 });

                  // Join the room using an alias.
                  bob->join_room("#" + alias + ":localhost",
                                 [](const nlohmann::json &, RequestErr err) { check_error(err); });
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

                bob->join_room(res.room_id, [room_id, bob](const nlohmann::json &, RequestErr err) {
                        check_error(err);

                        bob->leave_room(room_id, [](const nlohmann::json &, RequestErr err) {
                                check_error(err);
                        });
                });
        });

        // Trying to leave a non-existent room should fail.
        bob->leave_room(
          parse<Room>("!random_room_id:localhost"), [](const nlohmann::json &, RequestErr err) {
                  ASSERT_TRUE(err);
                  EXPECT_EQ(mtx::errors::to_string(err->matrix_error.errcode), "M_UNRECOGNIZED");
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
                  auto room_id = res.room_id;

                  alice->invite_user(room_id,
                                     "@bob:localhost",
                                     [room_id, bob](const mtx::responses::Empty &, RequestErr err) {
                                             check_error(err);

                                             bob->join_room(
                                               room_id, [](const nlohmann::json &, RequestErr err) {
                                                       check_error(err);
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
                  auto room_id = res.room_id;

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

                  mtx_client->sync(
                    "", "", false, 0, [](const mtx::responses::Sync &res, RequestErr err) {
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

                EXPECT_EQ(res.versions.size(), 4);
                EXPECT_EQ(res.versions.at(0), "r0.0.1");
                EXPECT_EQ(res.versions.at(1), "r0.1.0");
                EXPECT_EQ(res.versions.at(2), "r0.2.0");
                EXPECT_EQ(res.versions.at(3), "r0.3.0");
        });

        mtx_client->close();
}

TEST(ClientAPI, Typing)
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

                alice->start_typing(res.room_id, 10000, [alice, res](RequestErr err) {
                        check_error(err);

                        const auto room_id = res.room_id.to_string();
                        atomic_bool can_continue(false);

                        alice->sync("",
                                    "",
                                    false,
                                    0,
                                    [room_id, &can_continue](const mtx::responses::Sync &res,
                                                             RequestErr err) {
                                            check_error(err);

                                            can_continue = true;

                                            auto room = res.rooms.join.at(room_id);

                                            EXPECT_EQ(room.ephemeral.typing.size(), 1);
                                            EXPECT_EQ(room.ephemeral.typing.front(),
                                                      "@alice:localhost");
                                    });

                        while (!can_continue)
                                sleep();

                        alice->stop_typing(res.room_id, [alice, room_id](RequestErr err) {
                                check_error(err);

                                alice->sync(
                                  "",
                                  "",
                                  false,
                                  0,
                                  [room_id](const mtx::responses::Sync &res, RequestErr err) {
                                          check_error(err);
                                          auto room = res.rooms.join.at(room_id);
                                          EXPECT_EQ(room.ephemeral.typing.size(), 0);
                                  });
                        });
                });
        });

        alice->close();
}

TEST(ClientAPI, SendMessages)
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

                  bob->join_room(
                    res.room_id, [alice, bob, room_id](const nlohmann::json &, RequestErr err) {
                            check_error(err);

                            // Flag to indicate when those messages would be ready to be read by
                            // alice.
                            vector<string> event_ids;

                            mtx::events::msg::Text text;
                            text.body = "hello alice!";

                            bob->send_room_message<mtx::events::msg::Text,
                                                   mtx::events::EventType::RoomMessage>(
                              room_id,
                              text,
                              [&event_ids](const mtx::responses::EventId &res, RequestErr err) {
                                      event_ids.push_back(res.event_id.to_string());
                                      check_error(err);
                              });

                            mtx::events::msg::Emote emote;
                            emote.body = "*bob tests";

                            bob->send_room_message<mtx::events::msg::Emote,
                                                   mtx::events::EventType::RoomMessage>(
                              room_id,
                              emote,
                              [&event_ids](const mtx::responses::EventId &res, RequestErr err) {
                                      event_ids.push_back(res.event_id.to_string());
                                      check_error(err);
                              });

                            while (event_ids.size() != 2)
                                    sleep();

                            alice->sync(
                              "",
                              "",
                              false,
                              0,
                              [room_id, event_ids](const mtx::responses::Sync &res,
                                                   RequestErr err) {
                                      check_error(err);

                                      auto ids = get_event_ids<TimelineEvents>(
                                        res.rooms.join.at(room_id.to_string()).timeline.events);

                                      // The sent event ids should be visible in the timeline.
                                      for (const auto &event_id : event_ids)
                                              ASSERT_TRUE(std::find(ids.begin(),
                                                                    ids.end(),
                                                                    event_id) != std::end(ids));
                              });
                    });
          });

        alice->close();
        bob->close();
}

TEST(ClientAPI, SendStateEvents)
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

                  bob->send_state_event<mtx::events::state::Name, mtx::events::EventType::RoomName>(
                    room_id, event, [](const mtx::responses::EventId &, RequestErr err) {
                            ASSERT_TRUE(err);
                            ASSERT_EQ("M_FORBIDDEN",
                                      mtx::errors::to_string(err->matrix_error.errcode));
                    });

                  mtx::events::state::Name name_event;
                  name_event.name = "Alice's room";
                  alice
                    ->send_state_event<mtx::events::state::Name, mtx::events::EventType::RoomName>(
                      room_id,
                      name_event,
                      [&event_ids](const mtx::responses::EventId &res, RequestErr err) {
                              check_error(err);
                              event_ids.push_back(res.event_id.to_string());
                      });

                  mtx::events::state::Avatar avatar;
                  avatar.url = "mxc://localhost/random";
                  alice->send_state_event<mtx::events::state::Avatar,
                                          mtx::events::EventType::RoomAvatar>(
                    room_id,
                    avatar,
                    [&event_ids](const mtx::responses::EventId &res, RequestErr err) {
                            check_error(err);
                            event_ids.push_back(res.event_id.to_string());
                    });

                  while (event_ids.size() != 2)
                          sleep();

                  alice->sync(
                    "",
                    "",
                    false,
                    0,
                    [room_id, event_ids](const mtx::responses::Sync &res, RequestErr err) {
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

                alice->messages(
                  res.room_id,
                  "", // from
                  "", // to
                  PaginationDirection::Backwards,
                  30, // limit. Just enough messages so can fetch the whole history on
                      // this newly created room.
                  "", // filter
                  [room_id, alice](const mtx::responses::Messages &res, RequestErr err) {
                          check_error(err);

                          ASSERT_TRUE(res.chunk.size() > 5);
                          ASSERT_NE(res.start, res.end);

                          alice->messages(room_id,
                                          res.end,
                                          "",
                                          PaginationDirection::Backwards,
                                          30,
                                          "",
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

TEST(ClientAPI, ReadMarkers)
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

                alice
                  ->send_room_message<mtx::events::msg::Text, mtx::events::EventType::RoomMessage>(
                    room_id,
                    text,
                    [alice, &event_id, room_id](const mtx::responses::EventId &res,
                                                RequestErr err) {
                            check_error(err);

                            alice->read_event(
                              room_id, res.event_id, [&event_id, res](RequestErr err) {
                                      check_error(err);
                                      event_id = res.event_id.to_string();
                              });
                    });

                while (event_id.size() == 0)
                        sleep();

                alice->sync("",
                            "",
                            false,
                            0,
                            [room_id, event_id](const mtx::responses::Sync &res, RequestErr err) {
                                    check_error(err);

                                    auto receipts =
                                      res.rooms.join.at(room_id.to_string()).ephemeral.receipts;
                                    EXPECT_EQ(receipts.size(), 1);

                                    auto users = receipts[event_id];
                                    EXPECT_EQ(users.size(), 1);
                                    ASSERT_TRUE(users["@alice:localhost"] > 0);
                            });
        });

        alice->close();
}
