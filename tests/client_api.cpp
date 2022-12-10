#include <atomic>

#include <gtest/gtest.h>

#include <nlohmann/json.hpp>

#include "mtx/events/collections.hpp"
#include "mtx/events/encrypted.hpp"
#include "mtx/requests.hpp"
#include "mtx/responses.hpp"
#include "mtxclient/crypto/types.hpp"
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
    auto user = make_test_client();

    user->registration("alice",
                       "secret",
                       mtx::http::UIAHandler(
                         [](const mtx::http::UIAHandler &,
                            const mtx::user_interactive::Unauthorized &) { EXPECT_TRUE(false); }),
                       [](const mtx::responses::Register &, RequestErr err) {
                           ASSERT_TRUE(err);
                           EXPECT_EQ(mtx::errors::to_string(err->matrix_error.errcode),
                                     "M_USER_IN_USE");
                       });

    user->register_username_available(
      "alice", [](const mtx::responses::Available &, RequestErr err) {
          ASSERT_TRUE(err);
          EXPECT_EQ(err->matrix_error.errcode, mtx::errors::ErrorCode::M_USER_IN_USE);
      });

    auto username = utils::random_token(10, false);

    // Synapse converts the username to lowercase.
    for (auto &c : username) {
        c = (char)std::tolower(c);
    }

    user->register_username_available(
      username + "a", [](const mtx::responses::Available &available, RequestErr err) {
          check_error(err);
          EXPECT_TRUE(available.available);
      });

    user->registration(
      username,
      "secret",
      mtx::http::UIAHandler([](const mtx::http::UIAHandler &h,
                               const mtx::user_interactive::Unauthorized &unauthorized) {
          EXPECT_EQ(unauthorized.flows.size(), 1);
          EXPECT_EQ(unauthorized.flows[0].stages[0], "m.login.dummy");

          mtx::user_interactive::Auth auth{unauthorized.session,
                                           mtx::user_interactive::auth::Dummy{}};
          h.next(auth);
      }),
      [user, username](const mtx::responses::Register &res, RequestErr err) {
          if (!err || err->matrix_error.unauthorized.flows.size() == 0)
              return;

          check_error(err);
          const auto user_id = "@" + username + ":" + server_name();
          EXPECT_EQ(res.user_id.to_string(), user_id);

          EXPECT_EQ(user->user_id().to_string(), user_id);
          EXPECT_FALSE(user->access_token().empty());
          EXPECT_EQ(user->access_token(), res.access_token);
          EXPECT_FALSE(user->device_id().empty());
          EXPECT_EQ(user->device_id(), res.device_id);
      });

    user->registration([](const mtx::responses::Register &, RequestErr err) {
        ASSERT_TRUE(err);
        EXPECT_EQ(err->status_code, 401);
        EXPECT_FALSE(err->matrix_error.unauthorized.flows.empty());
    });

    user->close();
}

TEST(ClientAPI, LoginSuccess)
{
    std::shared_ptr<Client> mtx_client = make_test_client();

    mtx_client->login("alice", "secret", [](const mtx::responses::Login &res, RequestErr err) {
        check_error(err);
        validate_login("@alice:" + server_name(), res);
    });

    mtx_client->login("bob", "secret", [](const mtx::responses::Login &res, RequestErr err) {
        check_error(err);
        validate_login("@bob:" + server_name(), res);
    });

    mtx_client->login("carl", "secret", [](const mtx::responses::Login &res, RequestErr err) {
        check_error(err);
        validate_login("@carl:" + server_name(), res);
    });

    mtx_client->close();
}

TEST(ClientAPI, LoginWrongPassword)
{
    std::shared_ptr<Client> mtx_client = make_test_client();

    mtx_client->login(
      "alice", "wrong_password", [](const mtx::responses::Login &res, RequestErr err) {
          ASSERT_TRUE(err);
          EXPECT_EQ(mtx::errors::to_string(err->matrix_error.errcode), "M_FORBIDDEN");
          EXPECT_EQ(err->status_code, 403);

          EXPECT_EQ(res.user_id.to_string(), "");
          EXPECT_EQ(res.device_id, "");
          EXPECT_EQ(res.access_token, "");
      });

    mtx_client->close();
}

TEST(ClientAPI, LoginWrongUsername)
{
    std::shared_ptr<Client> mtx_client = make_test_client();

    mtx_client->login("john", "secret", [](const mtx::responses::Login &res, RequestErr err) {
        ASSERT_TRUE(err);
        EXPECT_EQ(mtx::errors::to_string(err->matrix_error.errcode), "M_FORBIDDEN");
        EXPECT_EQ(err->status_code, 403);

        EXPECT_EQ(res.user_id.to_string(), "");
        EXPECT_EQ(res.device_id, "");
        EXPECT_EQ(res.access_token, "");
    });

    mtx_client->close();
}

TEST(ClientAPI, LoginFlows)
{
    std::shared_ptr<Client> mtx_client = make_test_client();

    mtx_client->get_login([](const mtx::responses::LoginFlows &res, RequestErr err) {
        ASSERT_FALSE(err);

        EXPECT_EQ(res.flows[0].type, mtx::user_interactive::auth_types::password);
    });

    mtx_client->close();
}

TEST(ClientAPI, SSORedirect)
{
    std::shared_ptr<Client> mtx_client = std::make_shared<Client>("localhost", 443);
    EXPECT_EQ(mtx_client->login_sso_redirect("http://aaa:555/sso"),
              "https://localhost:443/_matrix/client/v3/login/sso/"
              "redirect?redirectUrl=http%3A%2F%2Faaa%3A555%2Fsso");
    mtx_client->close();
}

TEST(ClientAPI, EmptyUserAvatar)
{
    auto alice = make_test_client();

    alice->login("alice", "secret", [alice](const mtx::responses::Login &res, RequestErr err) {
        ASSERT_FALSE(err);

        auto const alice_id = res.user_id;
        validate_login(alice_id.to_string(), res);

        alice->set_avatar_url("", [alice, alice_id](RequestErr err) {
            ASSERT_FALSE(err);

            alice->get_profile(
              alice_id.to_string(),
              [alice, alice_id](const mtx::responses::Profile &res, RequestErr err) {
                  ASSERT_FALSE(err);
                  ASSERT_TRUE(res.avatar_url.size() == 0);

                  alice->get_avatar_url(alice_id.to_string(),
                                        [](const mtx::responses::AvatarUrl &res, RequestErr err) {
                                            ASSERT_FALSE(err);
                                            ASSERT_TRUE(res.avatar_url.size() == 0);
                                        });
              });
        });
    });

    alice->close();
}

TEST(ClientAPI, RealUserAvatar)
{
    auto alice = make_test_client();

    alice->login("alice", "secret", [alice](const mtx::responses::Login &res, RequestErr err) {
        ASSERT_FALSE(err);

        auto const alice_id   = res.user_id;
        auto const avatar_url = "mxc://matrix.org/wefh34uihSDRGhw34";

        validate_login(alice_id.to_string(), res);

        alice->set_avatar_url(avatar_url, [alice, alice_id, avatar_url](RequestErr err) {
            ASSERT_FALSE(err);

            alice->get_profile(
              alice_id.to_string(),
              [avatar_url, alice, alice_id](const mtx::responses::Profile &res, RequestErr err) {
                  ASSERT_FALSE(err);
                  ASSERT_TRUE(res.avatar_url == avatar_url);
                  alice->get_avatar_url(
                    alice_id.to_string(),
                    [avatar_url](const mtx::responses::AvatarUrl &res, RequestErr err) {
                        ASSERT_FALSE(err);
                        ASSERT_TRUE(res.avatar_url == avatar_url);
                    });
              });
        });
    });

    alice->close();
}

TEST(ClientAPI, ChangeDisplayName)
{
    std::shared_ptr<Client> mtx_client = make_test_client();

    mtx_client->login(
      "alice", "secret", [mtx_client](const mtx::responses::Login &, RequestErr err) {
          check_error(err);

          // Change the display name to Arthur Dent and verify its success through the lack
          // of an error
          mtx_client->set_displayname("Arthur Dent", [](RequestErr err) { check_error(err); });
      });

    mtx_client->close();
}

TEST(ClientAPI, EmptyDisplayName)
{
    std::shared_ptr<Client> mtx_client = make_test_client();

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
    std::shared_ptr<Client> mtx_client = make_test_client();

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
        EXPECT_EQ(res.room_id.hostname(), server_name());
    });

    mtx_client->close();
}

TEST(ClientAPI, CreateRoomInitialState)
{
    mtx::requests::CreateRoom req;

    mtx::events::StrippedEvent<mtx::events::state::Encryption> enc;
    enc.type                         = mtx::events::EventType::RoomEncryption;
    enc.content.algorithm            = mtx::crypto::MEGOLM_ALGO;
    enc.content.rotation_period_ms   = 1000ULL * 60ULL * 60ULL * 777ULL;
    enc.content.rotation_period_msgs = 777;

    req.initial_state.emplace_back(enc);

    std::shared_ptr<Client> mtx_client = make_test_client();

    mtx_client->login(
      "alice", "secret", [mtx_client](const mtx::responses::Login &, RequestErr err) {
          check_error(err);
      });

    while (mtx_client->access_token().empty())
        sleep();

    mtx_client->create_room(
      req, [mtx_client, enc](const mtx::responses::CreateRoom &res, RequestErr err) {
          check_error(err);
          ASSERT_TRUE(res.room_id.localpart().size() > 10);
          EXPECT_EQ(res.room_id.hostname(), server_name());

          mtx_client->get_state(
            res.room_id.to_string(), [enc](const mtx::responses::StateEvents &res, RequestErr err) {
                check_error(err);
                ASSERT_TRUE(res.events.size() > 0);
                bool found_enc_event = false;

                for (const auto &e : res.events) {
                    auto ev =
                      std::get_if<mtx::events::StateEvent<mtx::events::state::Encryption>>(&e);
                    if (ev) {
                        found_enc_event = true;
                        EXPECT_EQ(ev->content.algorithm, enc.content.algorithm);
                        EXPECT_EQ(ev->content.rotation_period_ms, enc.content.rotation_period_ms);
                        EXPECT_EQ(ev->content.rotation_period_msgs,
                                  enc.content.rotation_period_msgs);
                    }
                }
                EXPECT_TRUE(found_enc_event);
            });
      });

    mtx_client->close();
}

TEST(ClientAPI, Members)
{
    std::shared_ptr<Client> mtx_client = make_test_client();

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
          check_error(err);
          ASSERT_TRUE(res.room_id.localpart().size() > 10);
          EXPECT_EQ(res.room_id.hostname(), server_name());

          mtx_client->members(res.room_id.to_string(),
                              [](const mtx::responses::Members &res, RequestErr err) {
                                  check_error(err);
                                  ASSERT_EQ(res.chunk.size(), 1);
                                  EXPECT_EQ(res.chunk[0].state_key, "@alice:" + server_name());
                              });
      });

    mtx_client->close();
}

TEST(ClientAPI, TagRoom)
{
    std::shared_ptr<Client> mtx_client = make_test_client();

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
                  [mtx_client, room_id](mtx::events::account_data::Tags tags, RequestErr err) {
                      check_error(err);

                      EXPECT_EQ(tags.tags.at("u.Test").order, 0.5);

                      mtx_client->delete_tag(
                        room_id.to_string(), "u.Test", [mtx_client, room_id](RequestErr err) {
                            check_error(err);

                            mtx_client->get_tags(
                              room_id.to_string(),
                              [mtx_client, room_id](mtx::events::account_data::Tags tags,
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

TEST(ClientAPI, RoomAccountData)
{
    std::shared_ptr<Client> mtx_client = make_test_client();

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

          mtx::events::account_data::nheko_extensions::HiddenEvents hiddenEv;
          hiddenEv.hidden_event_types = std::vector{
            mtx::events::EventType::RoomMember,
          };

          mtx_client->put_room_account_data(
            room_id.to_string(), hiddenEv, [mtx_client, room_id](RequestErr err) {
                check_error(err);

                mtx_client->get_room_account_data<
                  mtx::events::account_data::nheko_extensions::HiddenEvents>(
                  room_id.to_string(),
                  [](mtx::events::account_data::nheko_extensions::HiddenEvents hiddenEv,
                     RequestErr err) {
                      check_error(err);

                      ASSERT_EQ(hiddenEv.hidden_event_types->size(), 1);
                      EXPECT_EQ(hiddenEv.hidden_event_types->at(0),
                                mtx::events::EventType::RoomMember);
                  });
            });
      });

    mtx_client->close();
}

TEST(ClientAPI, AccountData)
{
    std::shared_ptr<Client> mtx_client = make_test_client();

    mtx_client->login(
      "alice", "secret", [mtx_client](const mtx::responses::Login &, RequestErr err) {
          check_error(err);
      });

    while (mtx_client->access_token().empty())
        sleep();

    mtx::events::account_data::nheko_extensions::HiddenEvents hiddenEv;
    hiddenEv.hidden_event_types = std::vector{
      mtx::events::EventType::RoomMember,
    };

    mtx_client->put_account_data(hiddenEv, [mtx_client](RequestErr err) {
        check_error(err);

        mtx_client->get_account_data<mtx::events::account_data::nheko_extensions::HiddenEvents>(
          [mtx_client](mtx::events::account_data::nheko_extensions::HiddenEvents hiddenEv,
                       RequestErr err) {
              check_error(err);

              ASSERT_EQ(hiddenEv.hidden_event_types->size(), 1);
              EXPECT_EQ(hiddenEv.hidden_event_types->at(0), mtx::events::EventType::RoomMember);
          });
    });

    mtx_client->close();
}

TEST(ClientAPI, LogoutSuccess)
{
    std::shared_ptr<Client> mtx_client = make_test_client();
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
    std::shared_ptr<Client> mtx_client = make_test_client();
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
    auto alice = make_test_client();
    auto bob   = make_test_client();
    auto carl  = make_test_client();

    alice->login("alice", "secret", [alice](const mtx::responses::Login &, RequestErr err) {
        check_error(err);
    });

    bob->login(
      "bob", "secret", [bob](const mtx::responses::Login &, RequestErr err) { check_error(err); });

    carl->login("carl", "secret", [carl](const mtx::responses::Login &, RequestErr err) {
        check_error(err);
    });

    while (alice->access_token().empty() || bob->access_token().empty() ||
           carl->access_token().empty())
        sleep();

    mtx::requests::CreateRoom req;
    req.name   = "Name";
    req.topic  = "Topic";
    req.invite = {"@bob:" + server_name(), "@carl:" + server_name()};
    alice->create_room(req, [bob, carl](const mtx::responses::CreateRoom &res, RequestErr err) {
        check_error(err);
        auto room_id = res.room_id.to_string();

        bob->join_room(room_id,
                       [](const mtx::responses::RoomId &, RequestErr err) { check_error(err); });

        carl->join_room(room_id,
                        [](const mtx::responses::RoomId &, RequestErr err) { check_error(err); });
    });

    alice->close();
    bob->close();
    carl->close();
}

TEST(ClientAPI, JoinRoom)
{
    auto alice = make_test_client();
    auto bob   = make_test_client();

    alice->login("alice", "secret", [alice](const mtx::responses::Login &, RequestErr err) {
        check_error(err);
    });

    bob->login(
      "bob", "secret", [bob](const mtx::responses::Login &, RequestErr err) { check_error(err); });

    while (alice->access_token().empty() || bob->access_token().empty())
        sleep();

    // Creating a random room alias.
    // TODO: add a type for room aliases.
    const auto alias = utils::random_token(20, false);

    mtx::requests::CreateRoom req;
    req.name            = "Name";
    req.topic           = "Topic";
    req.invite          = {"@bob:" + server_name()};
    req.room_alias_name = alias;
    alice->create_room(req, [bob, alias](const mtx::responses::CreateRoom &res, RequestErr err) {
        check_error(err);
        auto room_id = res.room_id.to_string();

        bob->join_room(room_id,
                       [](const mtx::responses::RoomId &, RequestErr err) { check_error(err); });

        using namespace mtx::identifiers;
        bob->join_room(
          "!random_room_id:" + server_name(), [](const mtx::responses::RoomId &, RequestErr err) {
              ASSERT_TRUE(err);
              EXPECT_EQ(mtx::errors::to_string(err->matrix_error.errcode), "M_UNKNOWN");
          });

        // Join the room using an alias.
        bob->join_room("#" + alias + ":" + server_name(),
                       [](const mtx::responses::RoomId &, RequestErr err) { check_error(err); });
    });

    alice->close();
    bob->close();
}

TEST(ClientAPI, LeaveRoom)
{
    auto alice = make_test_client();
    auto bob   = make_test_client();

    alice->login("alice", "secret", [alice](const mtx::responses::Login &, RequestErr err) {
        check_error(err);
    });

    bob->login(
      "bob", "secret", [bob](const mtx::responses::Login &, RequestErr err) { check_error(err); });

    while (alice->access_token().empty() || bob->access_token().empty())
        sleep();

    mtx::requests::CreateRoom req;
    req.name   = "Name";
    req.topic  = "Topic";
    req.invite = {"@bob:" + server_name()};
    alice->create_room(req, [bob](const mtx::responses::CreateRoom &res, RequestErr err) {
        check_error(err);
        auto room_id = res.room_id;

        bob->join_room(
          res.room_id.to_string(), [room_id, bob](const mtx::responses::RoomId &, RequestErr err) {
              check_error(err);

              bob->leave_room(room_id.to_string(),
                              [](mtx::responses::Empty, RequestErr err) { check_error(err); });
          });
    });

    // Trying to leave a non-existent room should fail.
    bob->leave_room("!random_room_id:" + server_name(), [](mtx::responses::Empty, RequestErr err) {
        ASSERT_TRUE(err);
        EXPECT_EQ(mtx::errors::to_string(err->matrix_error.errcode), "M_UNKNOWN");
        EXPECT_EQ(err->matrix_error.error, "Not a known room");
    });

    alice->close();
    bob->close();
}

TEST(ClientAPI, InviteRoom)
{
    auto alice = make_test_client();
    auto bob   = make_test_client();

    alice->login("alice", "secret", [alice](const mtx::responses::Login &, RequestErr err) {
        check_error(err);
    });

    bob->login(
      "bob", "secret", [bob](const mtx::responses::Login &, RequestErr err) { check_error(err); });

    while (alice->access_token().empty() || bob->access_token().empty())
        sleep();

    mtx::requests::CreateRoom req;
    req.name   = "Name";
    req.topic  = "Topic";
    req.invite = {};
    alice->create_room(req, [alice, bob](const mtx::responses::CreateRoom &res, RequestErr err) {
        check_error(err);
        auto room_id = res.room_id.to_string();

        alice->invite_user(room_id,
                           "@bob:" + server_name(),
                           [room_id, bob](const mtx::responses::Empty &, RequestErr err) {
                               check_error(err);

                               bob->join_room(room_id,
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
    auto alice = make_test_client();
    auto bob   = make_test_client();

    alice->login("alice", "secret", [alice](const mtx::responses::Login &, RequestErr err) {
        check_error(err);
    });

    bob->login(
      "bob", "secret", [bob](const mtx::responses::Login &, RequestErr err) { check_error(err); });

    while (alice->access_token().empty() || bob->access_token().empty())
        sleep();

    mtx::requests::CreateRoom req;
    req.name   = "Name";
    req.topic  = "Topic";
    req.invite = {};
    alice->create_room(req, [alice, bob](const mtx::responses::CreateRoom &res, RequestErr err) {
        check_error(err);
        auto room_id = res.room_id.to_string();

        alice->invite_user(room_id,
                           "@bob:" + server_name(),
                           [room_id, alice, bob](const mtx::responses::Empty &, RequestErr err) {
                               check_error(err);

                               bob->join_room(
                                 room_id,
                                 [alice, room_id](const mtx::responses::RoomId &, RequestErr err) {
                                     check_error(err);

                                     alice->kick_user(room_id,
                                                      "@bob:" + server_name(),
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
    auto alice = make_test_client();
    auto bob   = make_test_client();

    alice->login("alice", "secret", [alice](const mtx::responses::Login &, RequestErr err) {
        check_error(err);
    });

    bob->login(
      "bob", "secret", [bob](const mtx::responses::Login &, RequestErr err) { check_error(err); });

    while (alice->access_token().empty() || bob->access_token().empty())
        sleep();

    mtx::requests::CreateRoom req;
    req.name   = "Name";
    req.topic  = "Topic";
    req.invite = {};
    alice->create_room(req, [alice, bob](const mtx::responses::CreateRoom &res, RequestErr err) {
        check_error(err);
        auto room_id = res.room_id.to_string();

        alice->invite_user(
          room_id,
          "@bob:" + server_name(),
          [room_id, alice, bob](const mtx::responses::Empty &, RequestErr err) {
              check_error(err);

              bob->join_room(
                room_id, [alice, room_id](const mtx::responses::RoomId &, RequestErr err) {
                    check_error(err);

                    alice->ban_user(
                      room_id,
                      "@bob:" + server_name(),
                      [alice, room_id](const mtx::responses::Empty &, RequestErr err) {
                          check_error(err);
                          alice->unban_user(
                            room_id,
                            "@bob:" + server_name(),
                            [](const mtx::responses::Empty &, RequestErr err) { check_error(err); },
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
    auto alice = make_test_client();
    auto bob   = make_test_client();

    alice->login("alice", "secret", [alice](const mtx::responses::Login &, RequestErr err) {
        check_error(err);
    });

    bob->login(
      "bob", "secret", [bob](const mtx::responses::Login &, RequestErr err) { check_error(err); });

    while (alice->access_token().empty() || bob->access_token().empty())
        sleep();

    mtx::requests::CreateRoom req;
    req.name   = "Name";
    req.topic  = "Topic";
    req.invite = {};
    alice->create_room(req, [alice, bob](const mtx::responses::CreateRoom &res, RequestErr err) {
        check_error(err);
        auto room_id = res.room_id.to_string();

        bob->invite_user(room_id,
                         "@carl:" + server_name(),
                         [room_id, bob](const mtx::responses::Empty &, RequestErr err) {
                             ASSERT_TRUE(err);
                             EXPECT_EQ(mtx::errors::to_string(err->matrix_error.errcode),
                                       "M_FORBIDDEN");
                         });
    });

    alice->close();
    bob->close();
}

TEST(ClientAPI, Sync)
{
    std::shared_ptr<Client> mtx_client = make_test_client();

    mtx_client->login(
      "alice", "secret", [mtx_client](const mtx::responses::Login &, RequestErr err) {
          check_error(err);
      });

    while (mtx_client->access_token().empty())
        sleep();

    mtx::requests::CreateRoom req;
    req.name  = "Name";
    req.topic = "Topic";
    mtx_client->create_room(req, [mtx_client](const mtx::responses::CreateRoom &, RequestErr err) {
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

TEST(ClientAPI, State)
{
    std::shared_ptr<Client> mtx_client = make_test_client();

    mtx_client->login(
      "alice", "secret", [mtx_client](const mtx::responses::Login &, RequestErr err) {
          check_error(err);
      });

    while (mtx_client->access_token().empty())
        sleep();

    mtx::requests::CreateRoom req;
    req.name  = "Name";
    req.topic = "Topic";
    mtx_client->create_room(req, [mtx_client](const mtx::responses::CreateRoom &r, RequestErr err) {
        check_error(err);

        mtx_client->get_state(
          r.room_id.to_string(), [](const mtx::responses::StateEvents &res, RequestErr err) {
              check_error(err);
              ASSERT_TRUE(res.events.size() > 0);
              bool found_name_event = false;

              for (const auto &e : res.events) {
                  auto ev = std::get_if<mtx::events::StateEvent<mtx::events::state::Name>>(&e);
                  if (ev && ev->content.name == "Name")
                      found_name_event = true;
              }
              EXPECT_TRUE(found_name_event);
          });
    });

    mtx_client->close();
}

TEST(ClientAPI, Versions)
{
    std::shared_ptr<Client> mtx_client = make_test_client();

    mtx_client->versions([](const mtx::responses::Versions &res, RequestErr err) {
        check_error(err);

        EXPECT_EQ(res.versions.size(), 10);
        EXPECT_EQ(res.versions.at(0), "r0.0.1");
        EXPECT_EQ(res.versions.at(1), "r0.1.0");
        EXPECT_EQ(res.versions.at(2), "r0.2.0");
        EXPECT_EQ(res.versions.at(3), "r0.3.0");
        EXPECT_EQ(res.versions.at(4), "r0.4.0");
        EXPECT_EQ(res.versions.at(5), "r0.5.0");
        EXPECT_EQ(res.versions.at(6), "r0.6.0");
        EXPECT_EQ(res.versions.at(7), "r0.6.1");
        EXPECT_EQ(res.versions.at(8), "v1.1");
        EXPECT_EQ(res.versions.at(9), "v1.2");
    });

    mtx_client->close();
}

TEST(ClientAPI, Capabilities)
{
    std::shared_ptr<Client> mtx_client = make_test_client();

    mtx_client->login(
      "alice", "secret", [mtx_client](const mtx::responses::Login &, RequestErr err) {
          check_error(err);
          mtx_client->capabilities(
            [](const mtx::responses::capabilities::Capabilities &res, RequestErr err) {
                check_error(err);

                EXPECT_GE(res.room_versions.default_.size(), 1);
                EXPECT_GE(res.room_versions.available.size(), 1);
                EXPECT_EQ(res.room_versions.available.at(res.room_versions.default_),
                          mtx::responses::capabilities::RoomVersionStability::Stable);
                EXPECT_EQ(res.change_3pid.enabled, true);
                EXPECT_EQ(res.change_password.enabled, true);
                EXPECT_EQ(res.set_avatar_url.enabled, true);
                EXPECT_EQ(res.set_displayname.enabled, true);
            });
      });

    mtx_client->close();
}

TEST(ClientAPI, Typing)
{
    auto alice = make_test_client();

    alice->login(
      "alice", "secret", [](const mtx::responses::Login &, RequestErr err) { check_error(err); });

    while (alice->access_token().empty())
        sleep();

    mtx::requests::CreateRoom req;
    alice->create_room(req, [alice](const mtx::responses::CreateRoom &res, RequestErr err) {
        check_error(err);

        alice->start_typing(res.room_id.to_string(), 10000, [alice, res](RequestErr err) {
            check_error(err);

            const auto room_id = res.room_id.to_string();

            SyncOpts opts;
            opts.timeout = 0;
            alice->sync(opts, [room_id, alice](const mtx::responses::Sync &res, RequestErr err) {
                check_error(err);

                auto room       = res.rooms.join.at(room_id);
                auto next_batch = res.next_batch;

                EXPECT_EQ(room.ephemeral.events.size(), 1);
                EXPECT_EQ(std::get<mtx::events::EphemeralEvent<mtx::events::ephemeral::Typing>>(
                            room.ephemeral.events.front())
                            .content.user_ids.front(),
                          "@alice:" + server_name());

                alice->stop_typing(room_id, [alice, room_id, next_batch](RequestErr err) {
                    check_error(err);

                    SyncOpts opts;
                    opts.timeout = 0;
                    opts.since   = next_batch;
                    alice->sync(opts, [room_id](const mtx::responses::Sync &res, RequestErr err) {
                        check_error(err);
                        auto room = res.rooms.join.at(room_id);
                        EXPECT_EQ(room.ephemeral.events.size(), 1);
                        EXPECT_EQ(
                          std::get<mtx::events::EphemeralEvent<mtx::events::ephemeral::Typing>>(
                            room.ephemeral.events.front())
                            .content.user_ids.size(),
                          0);
                    });
                });
            });
        });
    });

    alice->close();
}

TEST(ClientAPI, Presence)
{
    auto alice = make_test_client();

    alice->login(
      "alice", "secret", [](const mtx::responses::Login &, RequestErr err) { check_error(err); });

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
                        [alice](const mtx::events::presence::Presence &presence, RequestErr err) {
                            check_error(err);

                            EXPECT_EQ(presence.presence, mtx::presence::offline);
                            EXPECT_TRUE(presence.status_msg.empty());
                        });
                  });
            });
      });

    alice->close();
}

TEST(ClientAPI, PresenceOverSync)
{
    auto alice = make_test_client();
    auto bob   = make_test_client();

    alice->login(
      "alice", "secret", [](const mtx::responses::Login &, RequestErr err) { check_error(err); });
    bob->login(
      "bob", "secret", [](const mtx::responses::Login &, RequestErr err) { check_error(err); });

    while (alice->access_token().empty() || bob->access_token().empty())
        sleep();

    std::atomic<bool> can_exit = false;

    mtx::requests::CreateRoom req;
    req.invite = {"@bob:" + server_name()};
    alice->create_room(
      req, [alice, bob, &can_exit](const mtx::responses::CreateRoom &res, RequestErr err) {
          check_error(err);
          auto room_id = res.room_id.to_string();
          ASSERT_FALSE(room_id.empty());

          bob->join_room(
            room_id,
            [alice, bob, room_id, &can_exit](const mtx::responses::RoomId &, RequestErr err) {
                check_error(err);
                alice->put_presence_status(
                  mtx::presence::unavailable,
                  "Is this thing on?",
                  [alice, bob, &can_exit](RequestErr err) {
                      check_error(err);

                      SyncOpts opts;
                      opts.timeout      = 10;
                      opts.set_presence = mtx::presence::online;
                      alice->sync(
                        opts, [bob, opts, &can_exit](const mtx::responses::Sync &, RequestErr err) {
                            check_error(err);

                            bob->sync(
                              opts,
                              [bob, &can_exit](const mtx::responses::Sync &s, RequestErr err) {
                                  check_error(err);

                                  can_exit = true;

                                  ASSERT_GE(s.presence.size(), 1);

                                  bool found = false;
                                  for (const auto &p : s.presence) {
                                      if (p.sender == "@alice:" + server_name()) {
                                          found = true;
                                          EXPECT_EQ(p.content.presence, mtx::presence::online);
                                          EXPECT_EQ(p.content.status_msg,
                                                    "Is this thing "
                                                    "on?");
                                      }
                                  }
                                  EXPECT_TRUE(found);
                              });
                        });
                  });
            });
      });

    WAIT_UNTIL(can_exit);

    alice->close();
    bob->close();
}

TEST(ClientAPI, SendMessages)
{
    auto alice = make_test_client();
    auto bob   = make_test_client();

    alice->login("alice", "secret", [alice](const mtx::responses::Login &, RequestErr err) {
        check_error(err);
    });

    bob->login(
      "bob", "secret", [bob](const mtx::responses::Login &, RequestErr err) { check_error(err); });

    while (alice->access_token().empty() || bob->access_token().empty())
        sleep();

    mtx::requests::CreateRoom req;
    req.invite = {"@bob:" + server_name()};
    alice->create_room(req, [alice, bob](const mtx::responses::CreateRoom &res, RequestErr err) {
        check_error(err);
        auto room_id = res.room_id.to_string();

        bob->join_room(
          room_id, [alice, bob, room_id](const mtx::responses::RoomId &, RequestErr err) {
              check_error(err);

              mtx::events::msg::Text text;
              text.body = "hello alice!";

              bob->send_room_message<mtx::events::msg::Text>(
                room_id,
                text,
                [alice, bob, room_id](const mtx::responses::EventId &res, RequestErr err) {
                    auto evid1 = res.event_id.to_string();
                    check_error(err);

                    mtx::events::msg::Emote emote;
                    emote.body = "*bob tests";

                    bob->send_room_message<mtx::events::msg::Emote>(
                      room_id,
                      emote,
                      [alice, room_id, evid1](const mtx::responses::EventId &res, RequestErr err) {
                          auto evid2 = res.event_id.to_string();
                          check_error(err);

                          SyncOpts opts;
                          opts.timeout = 0;
                          alice->sync(opts,
                                      [room_id, evid1, evid2](const mtx::responses::Sync &res,
                                                              RequestErr err) {
                                          check_error(err);

                                          auto ids = get_event_ids<TimelineEvents>(
                                            res.rooms.join.at(room_id).timeline.events);

                                          // The sent event ids should be visible in
                                          // the timeline.
                                          ASSERT_TRUE(std::find(ids.begin(), ids.end(), evid1) !=
                                                      std::end(ids));
                                          ASSERT_TRUE(std::find(ids.begin(), ids.end(), evid2) !=
                                                      std::end(ids));
                                      });
                      });
                });
          });
    });

    bob->close();
    alice->close();
}

TEST(ClientAPI, RedactEvent)
{
    auto alice = make_test_client();
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
          room_id, text, [room_id, alice](const mtx::responses::EventId &res, RequestErr err) {
              check_error(err);

              alice->redact_event(room_id,
                                  res.event_id.to_string(),
                                  [](const mtx::responses::EventId &res, RequestErr err) {
                                      check_error(err);
                                      ASSERT_FALSE(res.event_id.to_string().empty());
                                  });
          });
    });

    alice->close();
}

TEST(ClientAPI, SendStateEvents)
{
    auto alice = make_test_client();
    auto bob   = make_test_client();

    alice->login("alice", "secret", [alice](const mtx::responses::Login &, RequestErr err) {
        check_error(err);
    });

    bob->login(
      "bob", "secret", [bob](const mtx::responses::Login &, RequestErr err) { check_error(err); });

    while (alice->access_token().empty() || bob->access_token().empty())
        sleep();

    mtx::requests::CreateRoom req;
    req.invite = {"@bob:" + server_name()};
    alice->create_room(req, [alice, bob](const mtx::responses::CreateRoom &res, RequestErr err) {
        check_error(err);
        auto room_id = res.room_id;

        mtx::events::state::Name event;
        event.name = "Bob's room";

        bob->send_state_event<mtx::events::state::Name>(
          room_id.to_string(), event, [](const mtx::responses::EventId &, RequestErr err) {
              ASSERT_TRUE(err);
              ASSERT_EQ("M_FORBIDDEN", mtx::errors::to_string(err->matrix_error.errcode));
          });

        mtx::events::state::Name name_event;
        name_event.name = "Alice's room";
        alice->send_state_event<mtx::events::state::Name>(
          room_id.to_string(),
          name_event,
          [alice, room_id](const mtx::responses::EventId &res, RequestErr err) {
              check_error(err);
              auto evid1 = res.event_id.to_string();

              mtx::events::state::Avatar avatar;
              avatar.url = "mxc://localhost/random";
              alice->send_state_event<mtx::events::state::Avatar>(
                room_id.to_string(),
                avatar,
                [alice, room_id, evid1](const mtx::responses::EventId &res, RequestErr err) {
                    check_error(err);
                    auto evid2 = res.event_id.to_string();

                    SyncOpts opts;
                    opts.timeout = 0;
                    alice->sync(
                      opts,
                      [room_id, evid1, evid2](const mtx::responses::Sync &res, RequestErr err) {
                          check_error(err);

                          auto ids = get_event_ids<TimelineEvents>(
                            res.rooms.join.at(room_id.to_string()).timeline.events);

                          // The sent event ids should be visible in the
                          // timeline.
                          ASSERT_TRUE(std::find(ids.begin(), ids.end(), evid1) != std::end(ids));
                          ASSERT_TRUE(std::find(ids.begin(), ids.end(), evid2) != std::end(ids));
                      });
                });
          });
    });

    alice->close();
    bob->close();
}

TEST(ClientAPI, GetStateEvents)
{
    auto alice = make_test_client();
    auto bob   = make_test_client();

    alice->login("alice", "secret", [alice](const mtx::responses::Login &, RequestErr err) {
        check_error(err);
    });

    bob->login(
      "bob", "secret", [bob](const mtx::responses::Login &, RequestErr err) { check_error(err); });

    while (alice->access_token().empty() || bob->access_token().empty())
        sleep();

    mtx::requests::CreateRoom req;
    // req.visibility = common::RoomVisibility::Public;
    req.name = "This is a test";
    alice->create_room(req, [alice, bob](const mtx::responses::CreateRoom &res, RequestErr err) {
        check_error(err);
        auto room_id = res.room_id;

        mtx::events::state::HistoryVisibility vis;
        vis.history_visibility = mtx::events::state::Visibility::WorldReadable;
        alice->send_state_event<mtx::events::state::HistoryVisibility>(
          room_id.to_string(),
          vis,
          [room_id, bob](const mtx::responses::EventId &, RequestErr err) {
              ASSERT_FALSE(err);

              bob->get_state_event<mtx::events::state::Name>(
                room_id.to_string(), "", [](const mtx::events::state::Name &name, RequestErr err) {
                    ASSERT_FALSE(err);

                    EXPECT_EQ(name.name, "This is a test");
                });
          });

        alice->get_state_event<mtx::events::state::Name>(
          room_id.to_string(), "", [](const mtx::events::state::Name &name, RequestErr err) {
              ASSERT_FALSE(err);

              EXPECT_EQ(name.name, "This is a test");
          });
    });

    alice->close();
    bob->close();
}
TEST(ClientAPI, Pagination)
{
    auto alice = make_test_client();

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
              alice->messages(opts, [](const mtx::responses::Messages &res, RequestErr err) {
                  check_error(err);

                  // We reached the start of the timeline.
                  // Old synapse versions send start == end in that case, newer ones send an empty
                  // token.
                  EXPECT_TRUE(res.start == res.end || res.end.empty());
                  EXPECT_EQ(res.chunk.size(), 0);
              });
          });
    });

    alice->close();
}

TEST(ClientAPI, UploadFilter)
{
    auto alice = make_test_client();

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
    auto alice = make_test_client();

    alice->login("alice", "secret", [alice](const mtx::responses::Login &, RequestErr err) {
        check_error(err);
    });

    while (alice->access_token().empty())
        sleep();

    mtx::requests::CreateRoom req;
    alice->create_room(req, [alice](const mtx::responses::CreateRoom &res, RequestErr err) {
        check_error(err);

        mtx::events::msg::Text text;
        text.body = "hello alice!";

        const auto room_id = res.room_id;

        alice->send_room_message<mtx::events::msg::Text>(
          room_id.to_string(),
          text,
          [alice, room_id](const mtx::responses::EventId &res, RequestErr err) {
              check_error(err);

              alice->read_event(
                room_id.to_string(),
                res.event_id.to_string(),
                [alice, room_id, res](RequestErr err) {
                    check_error(err);
                    auto event_id = res.event_id.to_string();

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
                          using mtx::events::ephemeral::Receipt;
                          EXPECT_EQ(users[Receipt::Read].users.size(), 1);
                          ASSERT_TRUE(users[Receipt::Read].users["@alice:" + server_name()].ts > 0);
                      });
                });
          });
    });

    alice->close();
}

TEST(ClientAPI, SendToDevice)
{
    auto alice = make_test_client();
    auto bob   = make_test_client();

    alice->login("alice", "secret", &check_login);
    bob->login("bob", "secret", &check_login);

    while (alice->access_token().empty() || bob->access_token().empty())
        sleep();

    nlohmann::json body{{"messages",
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

            auto event = std::get<mtx::events::DeviceEvent<mtx::events::msg::KeyRequest>>(
              res.to_device.events[0]);
            EXPECT_EQ(event.content.action, mtx::events::msg::RequestAction::Request);
            EXPECT_EQ(event.content.sender_key, "test");
            EXPECT_EQ(event.content.algorithm, "test_algo");
            EXPECT_EQ(event.content.room_id, "test_room_id");
            EXPECT_EQ(event.content.session_id, "test_session_id");
            EXPECT_EQ(event.content.request_id, "test_request_id");
            EXPECT_EQ(event.content.requesting_device_id, "test_req_id");
            EXPECT_EQ(event.type, mtx::events::EventType::RoomKeyRequest);
            EXPECT_EQ(event.sender, "@alice:" + server_name());
        });
    });

    alice->close();
    bob->close();
}

TEST(ClientAPI, NewSendToDevice)
{
    auto alice = make_test_client();
    auto bob   = make_test_client();
    auto carl  = make_test_client();

    alice->login("alice", "secret", &check_login);
    bob->login("bob", "secret", &check_login);
    carl->login("carl", "secret", &check_login);

    while (alice->access_token().empty() || bob->access_token().empty() ||
           carl->access_token().empty())
        sleep();

    ToDeviceMessages<mtx::events::msg::KeyRequest> body1;
    ToDeviceMessages<mtx::events::msg::KeyRequest> body2;

    mtx::events::msg::KeyRequest request1;

    request1.action               = mtx::events::msg::RequestAction::Request;
    request1.sender_key           = "test";
    request1.algorithm            = "m.megolm.v1.aes-sha2";
    request1.room_id              = "test_room_id";
    request1.session_id           = "test_session_id";
    request1.request_id           = "test_request_id";
    request1.requesting_device_id = "test_req_id";

    body1[bob->user_id()][bob->device_id()] = request1;

    mtx::events::msg::KeyRequest request2;

    request2.action               = mtx::events::msg::RequestAction::Cancellation;
    request2.request_id           = "test_request_id_1";
    request2.requesting_device_id = "test_req_id_1";

    body2[bob->user_id()][bob->device_id()] = request2;

    carl->send_to_device("m.room.key_request", body1, [bob, alice, body2](RequestErr err) {
        check_error(err);

        alice->send_to_device("m.room_key_request", body2, [bob](RequestErr err) {
            check_error(err);

            SyncOpts opts;
            opts.timeout = 0;
            bob->sync(opts, [](const mtx::responses::Sync &res, RequestErr err) {
                check_error(err);

                EXPECT_EQ(res.to_device.events.size(), 2);
                auto event = std::get<mtx::events::DeviceEvent<mtx::events::msg::KeyRequest>>(
                  res.to_device.events[0]);
            });
        });
    });

    carl->close();
    alice->close();
    bob->close();
}

TEST(ClientAPI, RetrieveSingleEvent)
{
    auto bob = make_test_client();
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
          room_id, text, [room_id, bob](const mtx::responses::EventId &res, RequestErr err) {
              check_error(err);

              bob->get_event(
                room_id,
                res.event_id.to_string(),
                [event_id = res.event_id.to_string()](
                  const mtx::events::collections::TimelineEvents &res, RequestErr err) {
                    check_error(err);
                    ASSERT_TRUE(
                      std::holds_alternative<mtx::events::RoomEvent<mtx::events::msg::Text>>(res));
                    auto e = std::get<mtx::events::RoomEvent<mtx::events::msg::Text>>(res);
                    EXPECT_EQ(e.content.body, "Hello Alice!");
                    EXPECT_EQ(e.sender, "@bob:" + server_name());
                    EXPECT_EQ(e.event_id, event_id);
                });

              bob->get_event(room_id,
                             "$random_event:" + server_name(),
                             [event_id = res.event_id.to_string()](
                               const mtx::events::collections::TimelineEvents &, RequestErr err) {
                                 ASSERT_TRUE(err);
                                 EXPECT_EQ(static_cast<int>(err->status_code), 404);
                             });
          });
    });

    bob->close();
}

TEST(ClientAPI, PublicRooms)
{
    // Setup : Create a new (public) room with some settings
    auto alice = make_test_client();
    auto bob   = make_test_client();

    alice->login("alice", "secret", [alice](const mtx::responses::Login &, RequestErr err) {
        check_error(err);
    });

    bob->login(
      "bob", "secret", [bob](const mtx::responses::Login &, RequestErr err) { check_error(err); });

    while (alice->access_token().empty() || bob->access_token().empty())
        sleep();

    std::string room_name = "Public Room" + alice->generate_txn_id();
    mtx::requests::CreateRoom req;
    req.name            = room_name;
    req.topic           = "Test";
    req.visibility      = mtx::common::RoomVisibility::Public;
    req.invite          = {"@bob:" + server_name()};
    req.room_alias_name = alice->generate_txn_id();
    req.preset          = Preset::PublicChat;

    std::atomic<bool> can_exit = false;

    alice->create_room(
      req,
      [&can_exit, alice, bob, room_name](const mtx::responses::CreateRoom &res, RequestErr err) {
          check_error(err);
          auto room_id = res.room_id;

          // TEST 1: endpoints to set and get the visibility of the room we
          // just created
          mtx::requests::PublicRoomVisibility r;
          r.visibility = mtx::common::RoomVisibility::Public;

          alice->put_room_visibility(
            room_id.to_string(), r, [&can_exit, alice, bob, room_id, room_name](RequestErr err) {
                check_error(err);

                // prevent unknown room error.
                std::this_thread::sleep_for(std::chrono::seconds(1));

                alice->get_room_visibility(
                  "",
                  [alice, room_id](const mtx::responses::PublicRoomVisibility &, RequestErr err) {
                      ASSERT_TRUE(err);
                      EXPECT_EQ(mtx::errors::to_string(err->matrix_error.errcode), "M_NOT_FOUND");
                  });

                alice->get_room_visibility(
                  room_id.to_string(),
                  [&can_exit, alice, bob, room_id, room_name](
                    const mtx::responses::PublicRoomVisibility &res, RequestErr err) {
                      check_error(err);
                      EXPECT_EQ(mtx::common::visibilityToString(res.visibility), "public");

                      // TEST 2: endpoints to add and list the
                      // public rooms on the server
                      mtx::requests::PublicRooms room_req;
                      room_req.limit                = 10;
                      room_req.include_all_networks = true;

                      alice->post_public_rooms(
                        room_req,
                        [&can_exit, alice, bob, room_id, room_req, room_name](
                          const mtx::responses::PublicRooms &, RequestErr err) {
                            check_error(err);

                            // wait for background update
                            std::this_thread::sleep_for(std::chrono::seconds(1));

                            alice->get_public_rooms(
                              [&can_exit, alice, bob, room_id, room_name](
                                const mtx::responses::PublicRooms &res, RequestErr err) {
                                  check_error(err);

                                  size_t idx = 0;
                                  for (const auto &c : res.chunk) {
                                      if (c.room_id == room_id.to_string())
                                          break;
                                      idx++;
                                  }

                                  if (idx >= res.chunk.size())
                                      ADD_FAILURE();
                                  else {
                                      EXPECT_EQ(res.chunk[idx].name, room_name);
                                      EXPECT_EQ(res.chunk[idx].topic, "Test");
                                      EXPECT_EQ(res.chunk[idx].num_joined_members, 1);
                                  }

                                  // Have bob join the
                                  // room and verify
                                  // there are 2 members
                                  bob->join_room(
                                    room_id.to_string(),
                                    [&can_exit, alice, bob, room_id](const mtx::responses::RoomId &,
                                                                     RequestErr err) {
                                        check_error(err);

                                        // wait for background update
                                        std::this_thread::sleep_for(std::chrono::seconds(1));

                                        alice->get_public_rooms(
                                          [&can_exit, alice, bob, room_id](
                                            const mtx::responses::PublicRooms &res,
                                            RequestErr err) {
                                              check_error(err);

                                              size_t idx = 0;
                                              for (const auto &c : res.chunk) {
                                                  if (c.room_id == room_id.to_string())
                                                      break;
                                                  idx++;
                                              }

                                              if (idx < res.chunk.size())
                                                  EXPECT_EQ(res.chunk[idx].num_joined_members, 2);
                                              else
                                                  ADD_FAILURE();

                                              // Teardown: remove
                                              // the room from the
                                              // room directory
                                              // (maintain future
                                              // tests)
                                              mtx::requests::PublicRoomVisibility r;
                                              r.visibility = mtx::common::RoomVisibility::Private;
                                              alice->put_room_visibility(
                                                room_id.to_string(),
                                                r,
                                                [&can_exit](RequestErr err) {
                                                    check_error(err);

                                                    can_exit = true;
                                                });
                                          },
                                          server_name(),
                                          1);
                                    });
                              },
                              server_name(),
                              1);
                        },
                        server_name());
                  });
            });
      });

    WAIT_UNTIL(can_exit);

    alice->close();
    bob->close();
}

TEST(ClientAPI, Users)
{
    auto alice = make_test_client();
    auto bob   = make_test_client();
    auto carl  = make_test_client();

    alice->login("alice", "secret", [alice](const mtx::responses::Login &, RequestErr err) {
        check_error(err);
    });

    bob->login("bob", "secret", [bob](const mtx::responses::Login &, RequestErr err) {
        check_error(err);
        bob->set_displayname("Bob", [](RequestErr err) { check_error(err); });
    });

    carl->login("carl", "secret", [carl](const mtx::responses::Login &, RequestErr err) {
        check_error(err);
        carl->set_displayname("Bobby", [](RequestErr err) { check_error(err); });
    });

    while (alice->access_token().empty() || bob->access_token().empty() ||
           carl->access_token().empty())
        sleep();

    mtx::requests::CreateRoom req;
    req.name   = "Name";
    req.topic  = "Topic";
    req.invite = {"@bob:" + server_name(), "@carl:" + server_name()};
    alice->create_room(req, [bob, carl](const mtx::responses::CreateRoom &res, RequestErr err) {
        check_error(err);
        auto room_id = res.room_id.to_string();

        bob->join_room(room_id,
                       [](const mtx::responses::RoomId &, RequestErr err) { check_error(err); });

        carl->join_room(room_id,
                        [](const mtx::responses::RoomId &, RequestErr err) { check_error(err); });
    });

    alice->search_user_directory("carl",
                                 [alice](const mtx::responses::Users &users, RequestErr err) {
                                     check_error(err);
                                     EXPECT_EQ(users.results.size(), 1);
                                     EXPECT_EQ(users.results[0].display_name, "Bobby");
                                     EXPECT_EQ(users.limited, false);
                                 });
    // synapse appears to return limit+1 results, this does not seem to
    // be spec compliant. To make the tests work, we pass 0 to get 1 result
    alice->search_user_directory(
      "Bob",
      [alice](const mtx::responses::Users &users, RequestErr err) {
          check_error(err);
          EXPECT_LE(users.results.size(), 1);
          EXPECT_EQ(users.limited, true);
      },
      0);
    alice->search_user_directory(
      "Bob",
      [alice](const mtx::responses::Users &users, RequestErr err) {
          check_error(err);
          EXPECT_EQ(users.results.size(), 2);
          EXPECT_EQ(users.limited, false);
      },
      -1);

    alice->close();
    bob->close();
    carl->close();
}

TEST(ClientAPI, Summary)
{
    // Setup : Create a new (public) room with some settings
    auto alice = make_test_client();
    auto bob   = make_test_client();

    alice->login("alice", "secret", [alice](const mtx::responses::Login &, RequestErr err) {
        check_error(err);
    });

    bob->login(
      "bob", "secret", [bob](const mtx::responses::Login &, RequestErr err) { check_error(err); });

    while (alice->access_token().empty() || bob->access_token().empty())
        sleep();

    std::string room_name = "Public Room" + alice->generate_txn_id();
    mtx::requests::CreateRoom req;
    req.name            = room_name;
    req.topic           = "Test";
    req.visibility      = mtx::common::RoomVisibility::Public;
    req.invite          = {"@bob:" + server_name()};
    req.room_alias_name = alice->generate_txn_id();
    req.preset          = Preset::PublicChat;

    alice->create_room(
      req, [alice, bob, room_name](const mtx::responses::CreateRoom &res, RequestErr err) {
          check_error(err);
          auto room_id = res.room_id;

          bob->get_summary(room_id.to_string(),
                           [room_name](const mtx::responses::PublicRoom &res, RequestErr err) {
                               check_error(err);
                               EXPECT_EQ(res.name, room_name);
                               EXPECT_NE(res.room_version, "1");
                               EXPECT_NE(res.room_version, "");
                               EXPECT_EQ(res.membership, mtx::events::state::Membership::Invite);
                           });
      });

    alice->close();
    bob->close();
}
