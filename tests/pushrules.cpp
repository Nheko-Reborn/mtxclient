#include <gtest/gtest.h>

#include <iostream>

#include "mtx/identifiers.hpp"
#include "mtx/requests.hpp"
#include "mtx/responses/create_room.hpp"
#include <mtx/pushrules.hpp>
#include <nlohmann/json.hpp>

#include "test_helpers.hpp"

using json = nlohmann::json;

namespace ns = mtx::pushrules;

using namespace mtx::http;

TEST(Pushrules, GlobalRuleset)
{
        json data = R"({
  "global": {
    "content": [
      {
        "actions": [
          "notify",
          {
            "set_tweak": "sound",
            "value": "default"
          },
          {
            "set_tweak": "highlight"
          }
        ],
        "default": true,
        "enabled": true,
        "pattern": "alice",
        "rule_id": ".m.rule.contains_user_name"
      }
    ],
    "override": [
      {
        "actions": [
          "dont_notify"
        ],
        "conditions": [],
        "default": true,
        "enabled": false,
        "rule_id": ".m.rule.master"
      },
      {
        "actions": [
          "dont_notify"
        ],
        "conditions": [
          {
            "key": "content.msgtype",
            "kind": "event_match",
            "pattern": "m.notice"
          }
        ],
        "default": true,
        "enabled": true,
        "rule_id": ".m.rule.suppress_notices"
      }
    ],
    "room": [],
    "sender": [],
    "underride": [
      {
        "actions": [
          "notify",
          {
            "set_tweak": "sound",
            "value": "ring"
          },
          {
            "set_tweak": "highlight",
            "value": false
          }
        ],
        "conditions": [
          {
            "key": "type",
            "kind": "event_match",
            "pattern": "m.call.invite"
          }
        ],
        "default": true,
        "enabled": true,
        "rule_id": ".m.rule.call"
      },
      {
        "actions": [
          "notify",
          {
            "set_tweak": "sound",
            "value": "default"
          },
          {
            "set_tweak": "highlight"
          }
        ],
        "conditions": [
          {
            "kind": "contains_display_name"
          }
        ],
        "default": true,
        "enabled": true,
        "rule_id": ".m.rule.contains_display_name"
      },
      {
        "actions": [
          "notify",
          {
            "set_tweak": "sound",
            "value": "default"
          },
          {
            "set_tweak": "highlight",
            "value": false
          }
        ],
        "conditions": [
          {
            "kind": "room_member_count",
            "is": "2"
          },
          {
            "kind": "event_match",
            "key": "type",
            "pattern": "m.room.message"
          }
        ],
        "default": true,
        "enabled": true,
        "rule_id": ".m.rule.room_one_to_one"
      },
      {
        "actions": [
          "notify",
          {
            "set_tweak": "sound",
            "value": "default"
          },
          {
            "set_tweak": "highlight",
            "value": false
          }
        ],
        "conditions": [
          {
            "key": "type",
            "kind": "event_match",
            "pattern": "m.room.member"
          },
          {
            "key": "content.membership",
            "kind": "event_match",
            "pattern": "invite"
          },
          {
            "key": "state_key",
            "kind": "event_match",
            "pattern": "@alice:example.com"
          }
        ],
        "default": true,
        "enabled": true,
        "rule_id": ".m.rule.invite_for_me"
      },
      {
        "actions": [
          "notify",
          {
            "set_tweak": "highlight",
            "value": false
          }
        ],
        "conditions": [
          {
            "key": "type",
            "kind": "event_match",
            "pattern": "m.room.member"
          }
        ],
        "default": true,
        "enabled": true,
        "rule_id": ".m.rule.member_event"
      },
      {
        "actions": [
          "notify",
          {
            "set_tweak": "highlight",
            "value": false
          }
        ],
        "conditions": [
          {
            "key": "type",
            "kind": "event_match",
            "pattern": "m.room.message"
          }
        ],
        "default": true,
        "enabled": true,
        "rule_id": ".m.rule.message"
      }
    ]
  }
})"_json;

        ns::GlobalRuleset rules = data;
        EXPECT_EQ(rules.global.content.at(0).actions.size(), 3);
        EXPECT_TRUE(
          std::holds_alternative<ns::actions::notify>(rules.global.content.at(0).actions.at(0)));
        EXPECT_EQ(
          std::get<ns::actions::set_tweak_sound>(rules.global.content.at(0).actions.at(1)).value,
          "default");
        EXPECT_TRUE(std::holds_alternative<ns::actions::set_tweak_highlight>(
          rules.global.content.at(0).actions.at(2)));

        // EXPECT_EQ(rules.global.override_.size(), 2);
        EXPECT_EQ(rules.global.room.size(), 0);
        EXPECT_EQ(rules.global.sender.size(), 0);
        EXPECT_EQ(rules.global.underride.size(), 6);
        EXPECT_EQ(rules.global.underride[0].conditions.at(0).key, "type");
}

TEST(Pushrules, GetGlobalRuleset)
{
        std::shared_ptr<Client> client = make_test_client();

        client->login(
          "alice", "secret", [client](const mtx::responses::Login &res, RequestErr err) {
                  check_error(err);
                  validate_login("@alice:localhost", res);

                  client->get_pushrules([](const mtx::pushrules::GlobalRuleset &, RequestErr err) {
                          EXPECT_TRUE(!err);
                  });
          });
        client->close();
}

TEST(Pushrules, GetRuleset)
{
        std::shared_ptr<Client> client = make_test_client();

        client->login(
          "alice", "secret", [client](const mtx::responses::Login &res, RequestErr err) {
                  check_error(err);
                  validate_login("@alice:localhost", res);

                  client->get_pushrules("global",
                                        "content",
                                        ".m.rule.contains_user_name",
                                        [](const mtx::pushrules::PushRule &rule, RequestErr err) {
                                                EXPECT_TRUE(!err);
                                                EXPECT_EQ(rule.pattern, "alice");
                                        });
          });
        client->close();
}

TEST(Pushrules, PutAndDeleteRuleset)
{
        std::shared_ptr<Client> client = make_test_client();

        client->login(
          "alice", "secret", [client](const mtx::responses::Login &res, RequestErr err) {
                  check_error(err);
                  validate_login("@alice:localhost", res);

                  mtx::pushrules::PushRule rule;
                  rule.pattern = "cake";
                  rule.actions = {mtx::pushrules::actions::notify{},
                                  mtx::pushrules::actions::set_tweak_sound{"cakealarm.wav"}};
                  client->put_pushrules("global",
                                        "content",
                                        "SSByZWFsbHkgbGlrZSBjYWtl",
                                        rule,
                                        [client](RequestErr err) {
                                                EXPECT_TRUE(!err);

                                                client->delete_pushrules(
                                                  "global",
                                                  "content",
                                                  "SSByZWFsbHkgbGlrZSBjYWtl",
                                                  [](RequestErr err) { EXPECT_TRUE(!err); });
                                        });
          });
        client->close();
}

TEST(Pushrules, RulesetEnabled)
{
        std::shared_ptr<Client> client = make_test_client();

        client->login(
          "alice", "secret", [client](const mtx::responses::Login &res, RequestErr err) {
                  check_error(err);
                  validate_login("@alice:localhost", res);

                  client->get_pushrules_enabled(
                    "global",
                    "content",
                    ".m.rule.contains_user_name",
                    [client](const mtx::pushrules::Enabled &enabled, RequestErr err) {
                            EXPECT_TRUE(!err);

                            EXPECT_TRUE(enabled.enabled);
                            client->put_pushrules_enabled(
                              "global",
                              "content",
                              ".m.rule.contains_user_name",
                              false,
                              [client](RequestErr err) {
                                      EXPECT_TRUE(!err);
                                      client->get_pushrules_enabled(
                                        "global",
                                        "content",
                                        ".m.rule.contains_user_name",
                                        [client](const mtx::pushrules::Enabled &enabled,
                                                 RequestErr err) {
                                                EXPECT_TRUE(!err);

                                                EXPECT_FALSE(enabled.enabled);
                                                client->put_pushrules_enabled(
                                                  "global",
                                                  "content",
                                                  ".m.rule.contains_"
                                                  "user_name",
                                                  true,
                                                  [](RequestErr err) { EXPECT_TRUE(!err); });
                                        });
                              });
                    });
          });
        client->close();
}

TEST(Pushrules, Actions)
{
        std::shared_ptr<Client> client = make_test_client();

        client->login(
          "alice", "secret", [client](const mtx::responses::Login &res, RequestErr err) {
                  check_error(err);
                  validate_login("@alice:localhost", res);

                  mtx::pushrules::actions::Actions actions = {
                    {mtx::pushrules::actions::notify{},
                     mtx::pushrules::actions::set_tweak_sound{"cakealarm.wav"},
                     mtx::pushrules::actions::set_tweak_highlight{}}};
                  client->put_pushrules_actions("global",
                                                "content",
                                                ".m.rule.contains_user_name",
                                                actions,
                                                [client](RequestErr err) {
                                                        EXPECT_TRUE(!err);

                                                        client->get_pushrules_actions(
                                                          "global",
                                                          "content",
                                                          ".m.rule.contains_user_name",
                                                          [client](auto actions_, RequestErr err) {
                                                                  EXPECT_TRUE(!err);
                                                                  EXPECT_EQ(actions_.actions.size(),
                                                                            3);
                                                          });
                                                });
          });
        client->close();
}

TEST(Pushrules, RoomRuleMute)
{
        std::shared_ptr<Client> client = make_test_client();

        client->login(
          "alice", "secret", [client](const mtx::responses::Login &res, RequestErr err) {
                  check_error(err);
                  validate_login("@alice:localhost", res);

                  mtx::requests::CreateRoom req;
                  req.name  = "Name";
                  req.topic = "Topic";

                  client->create_room(
                    req, [client](const mtx::responses::CreateRoom &res, RequestErr err) {
                            check_error(err);
                            ASSERT_TRUE(res.room_id.localpart().size() > 10);
                            EXPECT_EQ(res.room_id.hostname(), "localhost");

                            mtx::pushrules::PushRule rule;
                            rule.actions = {mtx::pushrules::actions::dont_notify{}};
                            mtx::pushrules::PushCondition condition;
                            condition.kind    = "event_match";
                            condition.key     = "room_id";
                            condition.pattern = res.room_id.to_string();
                            rule.conditions   = {condition};

                            client->put_pushrules("global",
                                                  "override",
                                                  res.room_id.to_string(),
                                                  rule,
                                                  [](mtx::http::RequestErr &err) {
                                                          check_error(err);
                                                          EXPECT_TRUE(!err);
                                                  });
                    });
          });
        client->close();
}

TEST(Pushrules, RoomRuleMentions)
{
        std::shared_ptr<Client> client = make_test_client();

        client->login(
          "alice", "secret", [client](const mtx::responses::Login &res, RequestErr err) {
                  check_error(err);
                  validate_login("@alice:localhost", res);

                  mtx::requests::CreateRoom req;
                  req.name  = "Name";
                  req.topic = "Topic";

                  client->create_room(
                    req, [client](const mtx::responses::CreateRoom &res, RequestErr err) {
                            check_error(err);
                            ASSERT_TRUE(res.room_id.localpart().size() > 10);
                            EXPECT_EQ(res.room_id.hostname(), "localhost");

                            mtx::pushrules::PushRule rule;
                            rule.actions = {mtx::pushrules::actions::dont_notify{}};

                            client->put_pushrules("global",
                                                  "room",
                                                  res.room_id.to_string(),
                                                  rule,
                                                  [](mtx::http::RequestErr &err) {
                                                          check_error(err);
                                                          EXPECT_TRUE(!err);
                                                  });
                    });
          });
        client->close();
}
