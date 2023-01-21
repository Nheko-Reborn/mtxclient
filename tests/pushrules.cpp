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
        "pattern": "alice",
        "rule_id": ".m.rule.contains_user_name"
      }
    ],
    "override": [
      {
        "actions": [
          "dont_notify"
        ],
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
        "rule_id": ".m.rule.message"
      }
    ]
  }
})"_json;

    ns::GlobalRuleset rules = data.get<ns::GlobalRuleset>();
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
    EXPECT_EQ(rules.global.content[0].rule_id, ".m.rule.contains_user_name");
    EXPECT_EQ(data, json(rules));
}

TEST(Pushrules, GetGlobalRuleset)
{
    std::shared_ptr<Client> client = make_test_client();

    client->login("alice", "secret", [client](const mtx::responses::Login &res, RequestErr err) {
        check_error(err);
        validate_login("@alice:" + server_name(), res);

        client->get_pushrules(
          [](const mtx::pushrules::GlobalRuleset &, RequestErr err) { EXPECT_TRUE(!err); });
    });
    client->close();
}

TEST(Pushrules, GetRuleset)
{
    std::shared_ptr<Client> client = make_test_client();

    client->login("alice", "secret", [client](const mtx::responses::Login &res, RequestErr err) {
        check_error(err);
        validate_login("@alice:" + server_name(), res);

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

    client->login("alice", "secret", [client](const mtx::responses::Login &res, RequestErr err) {
        check_error(err);
        validate_login("@alice:" + server_name(), res);

        mtx::pushrules::PushRule rule;
        rule.pattern = "cake";
        rule.actions = {mtx::pushrules::actions::notify{},
                        mtx::pushrules::actions::set_tweak_sound{"cakealarm.wav"}};
        client->put_pushrules(
          "global", "content", "SSByZWFsbHkgbGlrZSBjYWtl", rule, [client](RequestErr err) {
              EXPECT_TRUE(!err);

              client->delete_pushrules("global",
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

    client->login("alice", "secret", [client](const mtx::responses::Login &res, RequestErr err) {
        check_error(err);
        validate_login("@alice:" + server_name(), res);

        client->get_pushrules_enabled(
          "global",
          "content",
          ".m.rule.contains_user_name",
          [client](const mtx::pushrules::Enabled &enabled, RequestErr err) {
              EXPECT_TRUE(!err);

              EXPECT_TRUE(enabled.enabled);
              client->put_pushrules_enabled(
                "global", "content", ".m.rule.contains_user_name", false, [client](RequestErr err) {
                    EXPECT_TRUE(!err);
                    client->get_pushrules_enabled(
                      "global",
                      "content",
                      ".m.rule.contains_user_name",
                      [client](const mtx::pushrules::Enabled &enabled, RequestErr err) {
                          EXPECT_TRUE(!err);

                          EXPECT_FALSE(enabled.enabled);
                          client->put_pushrules_enabled("global",
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

    client->login("alice", "secret", [client](const mtx::responses::Login &res, RequestErr err) {
        check_error(err);
        validate_login("@alice:" + server_name(), res);

        mtx::pushrules::actions::Actions actions = {
          {mtx::pushrules::actions::notify{},
           mtx::pushrules::actions::set_tweak_sound{"cakealarm.wav"},
           mtx::pushrules::actions::set_tweak_highlight{}}};
        client->put_pushrules_actions(
          "global", "content", ".m.rule.contains_user_name", actions, [client](RequestErr err) {
              EXPECT_TRUE(!err);

              client->get_pushrules_actions("global",
                                            "content",
                                            ".m.rule.contains_user_name",
                                            [client](auto actions_, RequestErr err) {
                                                EXPECT_TRUE(!err);
                                                EXPECT_EQ(actions_.actions.size(), 3);
                                            });
          });
    });
    client->close();
}

TEST(Pushrules, RoomRuleMute)
{
    std::shared_ptr<Client> client = make_test_client();

    client->login("alice", "secret", [client](const mtx::responses::Login &res, RequestErr err) {
        check_error(err);
        validate_login("@alice:" + server_name(), res);

        mtx::requests::CreateRoom req;
        req.name  = "Name";
        req.topic = "Topic";

        client->create_room(req, [client](const mtx::responses::CreateRoom &res, RequestErr err) {
            check_error(err);
            ASSERT_TRUE(res.room_id.localpart().size() > 10);
            EXPECT_EQ(res.room_id.hostname(), server_name());

            mtx::pushrules::PushRule rule;
            rule.actions = {mtx::pushrules::actions::dont_notify{}};
            mtx::pushrules::PushCondition condition;
            condition.kind    = "event_match";
            condition.key     = "room_id";
            condition.pattern = res.room_id.to_string();
            rule.conditions   = {condition};

            client->put_pushrules(
              "global", "override", res.room_id.to_string(), rule, [](mtx::http::RequestErr &err) {
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

    client->login("alice", "secret", [client](const mtx::responses::Login &res, RequestErr err) {
        check_error(err);
        validate_login("@alice:" + server_name(), res);

        mtx::requests::CreateRoom req;
        req.name  = "Name";
        req.topic = "Topic";

        client->create_room(req, [client](const mtx::responses::CreateRoom &res, RequestErr err) {
            check_error(err);
            ASSERT_TRUE(res.room_id.localpart().size() > 10);
            EXPECT_EQ(res.room_id.hostname(), server_name());

            mtx::pushrules::PushRule rule;
            rule.actions = {mtx::pushrules::actions::dont_notify{}};

            client->put_pushrules(
              "global", "room", res.room_id.to_string(), rule, [](mtx::http::RequestErr &err) {
                  check_error(err);
                  EXPECT_TRUE(!err);
              });
        });
    });
    client->close();
}

TEST(Pushrules, EventMatches)
{
    mtx::pushrules::PushRule event_match_rule;
    event_match_rule.actions = {
      mtx::pushrules::actions::notify{},
      mtx::pushrules::actions::set_tweak_highlight{},
    };
    event_match_rule.conditions.push_back(mtx::pushrules::PushCondition{
      .kind    = "event_match",
      .key     = "content.body",
      .pattern = "honk",
      .is      = "",
    });

    mtx::events::RoomEvent<mtx::events::msg::Text> textEv{};
    textEv.content.body = "abc def ghi honk jkl";
    textEv.room_id      = "!abc:def.ghi";
    textEv.event_id     = "$abc1234567890:def.ghi";
    textEv.sender       = "@me:def.ghi";

    auto testEval = [actions = event_match_rule.actions,
                     &textEv](const mtx::pushrules::PushRuleEvaluator &evaluator) {
        mtx::pushrules::PushRuleEvaluator::RoomContext ctx{};
        EXPECT_EQ(evaluator.evaluate({textEv}, ctx, {}), actions);

        auto textEvEnd         = textEv;
        textEvEnd.content.body = "abc honk";
        EXPECT_EQ(evaluator.evaluate({textEvEnd}, ctx, {}), actions);
        auto textEvStart         = textEv;
        textEvStart.content.body = "honk abc";
        EXPECT_EQ(evaluator.evaluate({textEvStart}, ctx, {}), actions);
        auto textEvNL         = textEv;
        textEvNL.content.body = "abc\nhonk\nabc";
        EXPECT_EQ(evaluator.evaluate({textEvNL}, ctx, {}), actions);
        auto textEvFull         = textEv;
        textEvFull.content.body = "honk";
        EXPECT_EQ(evaluator.evaluate({textEvFull}, ctx, {}), actions);
        auto textEvCase         = textEv;
        textEvCase.content.body = "HoNk";
        EXPECT_EQ(evaluator.evaluate({textEvCase}, ctx, {}), actions);
        auto textEvNo         = textEv;
        textEvNo.content.body = "HoN";
        EXPECT_TRUE(evaluator.evaluate({textEvNo}, ctx, {}).empty());
        auto textEvNo2         = textEv;
        textEvNo2.content.body = "honkb";
        EXPECT_TRUE(evaluator.evaluate({textEvNo2}, ctx, {}).empty());
        auto textEvWordBoundaries         = textEv;
        textEvWordBoundaries.content.body = "@honk:";
        EXPECT_EQ(evaluator.evaluate({textEvWordBoundaries}, ctx, {}), actions);

        // It is what the spec says ¯\_(ツ)_/¯
        auto textEvWordBoundaries2         = textEv;
        textEvWordBoundaries2.content.body = "ähonkü";
        EXPECT_EQ(evaluator.evaluate({textEvWordBoundaries2}, ctx, {}), actions);
    };

    mtx::pushrules::Ruleset override_ruleset;
    override_ruleset.override_.push_back(event_match_rule);
    mtx::pushrules::PushRuleEvaluator over_evaluator{override_ruleset};
    testEval(over_evaluator);

    mtx::pushrules::Ruleset underride_ruleset;
    underride_ruleset.underride.push_back(event_match_rule);
    mtx::pushrules::PushRuleEvaluator under_evaluator{underride_ruleset};
    testEval(under_evaluator);

    mtx::pushrules::Ruleset room_ruleset;
    auto room_rule    = event_match_rule;
    room_rule.rule_id = "!abc:def.ghi";
    room_ruleset.room.push_back(room_rule);
    mtx::pushrules::PushRuleEvaluator room_evaluator{room_ruleset};
    EXPECT_EQ(room_evaluator.evaluate({textEv}, {}, {}), room_rule.actions);

    mtx::pushrules::Ruleset sender_ruleset;
    auto sender_rule    = event_match_rule;
    sender_rule.rule_id = "@me:def.ghi";
    sender_ruleset.sender.push_back(sender_rule);
    mtx::pushrules::PushRuleEvaluator sender_evaluator{sender_ruleset};
    EXPECT_EQ(sender_evaluator.evaluate({textEv}, {}, {}), sender_rule.actions);

    mtx::pushrules::Ruleset content_ruleset;
    mtx::pushrules::PushRule content_match_rule;
    content_match_rule.actions = event_match_rule.actions;
    content_match_rule.pattern = "honk";
    content_ruleset.content.push_back(content_match_rule);
    mtx::pushrules::PushRuleEvaluator content_evaluator{content_ruleset};
    testEval(content_evaluator);
}

TEST(Pushrules, DisplaynameMatches)
{
    mtx::pushrules::PushRule event_match_rule;
    event_match_rule.actions = {
      mtx::pushrules::actions::notify{},
      mtx::pushrules::actions::set_tweak_highlight{},
    };
    event_match_rule.conditions.push_back(mtx::pushrules::PushCondition{
      .kind    = "contains_display_name",
      .key     = "",
      .pattern = "",
      .is      = "",
    });

    mtx::events::RoomEvent<mtx::events::msg::Text> textEv{};
    textEv.content.body = "abc def ghi honk jkl";
    textEv.room_id      = "!abc:def.ghi";
    textEv.event_id     = "$abc1234567890:def.ghi";
    textEv.sender       = "@me:def.ghi";

    auto testEval = [actions = event_match_rule.actions,
                     &textEv](const mtx::pushrules::PushRuleEvaluator &evaluator) {
        mtx::pushrules::PushRuleEvaluator::RoomContext ctx{};
        ctx.user_display_name = "honk";

        EXPECT_EQ(evaluator.evaluate({textEv}, ctx, {}), actions);

        auto textEvEnd         = textEv;
        textEvEnd.content.body = "abc honk";
        EXPECT_EQ(evaluator.evaluate({textEvEnd}, ctx, {}), actions);
        auto textEvStart         = textEv;
        textEvStart.content.body = "honk abc";
        EXPECT_EQ(evaluator.evaluate({textEvStart}, ctx, {}), actions);
        auto textEvNL         = textEv;
        textEvNL.content.body = "abc\nhonk\nabc";
        EXPECT_EQ(evaluator.evaluate({textEvNL}, ctx, {}), actions);
        auto textEvFull         = textEv;
        textEvFull.content.body = "honk";
        EXPECT_EQ(evaluator.evaluate({textEvFull}, ctx, {}), actions);
        auto textEvCase         = textEv;
        textEvCase.content.body = "HoNk";
        EXPECT_EQ(evaluator.evaluate({textEvCase}, ctx, {}), actions);
        auto textEvNo         = textEv;
        textEvNo.content.body = "HoN";
        EXPECT_TRUE(evaluator.evaluate({textEvNo}, ctx, {}).empty());
        auto textEvNo2         = textEv;
        textEvNo2.content.body = "honkb";
        EXPECT_TRUE(evaluator.evaluate({textEvNo2}, ctx, {}).empty());
        auto textEvWordBoundaries         = textEv;
        textEvWordBoundaries.content.body = "@honk:";
        EXPECT_EQ(evaluator.evaluate({textEvWordBoundaries}, ctx, {}), actions);

        // It is what the spec says ¯\_(ツ)_/¯
        auto textEvWordBoundaries2         = textEv;
        textEvWordBoundaries2.content.body = "ähonkü";
        EXPECT_EQ(evaluator.evaluate({textEvWordBoundaries2}, ctx, {}), actions);

        // assert empty displayname does not match
        mtx::pushrules::PushRuleEvaluator::RoomContext ctxEmpty{};
        EXPECT_TRUE(evaluator.evaluate({textEvWordBoundaries}, ctxEmpty, {}).empty());
    };

    mtx::pushrules::Ruleset override_ruleset;
    override_ruleset.override_.push_back(event_match_rule);
    mtx::pushrules::PushRuleEvaluator over_evaluator{override_ruleset};
    testEval(over_evaluator);

    mtx::pushrules::Ruleset underride_ruleset;
    underride_ruleset.underride.push_back(event_match_rule);
    mtx::pushrules::PushRuleEvaluator under_evaluator{underride_ruleset};
    testEval(under_evaluator);
}

TEST(Pushrules, PowerLevelMatches)
{
    mtx::pushrules::PushRule event_match_rule;
    event_match_rule.actions = {
      mtx::pushrules::actions::notify{},
      mtx::pushrules::actions::set_tweak_highlight{},
    };
    event_match_rule.conditions.push_back(mtx::pushrules::PushCondition{
      .kind    = "sender_notification_permission",
      .key     = "room",
      .pattern = "",
      .is      = "",
    });

    auto testEval =
      [actions = event_match_rule.actions](const mtx::pushrules::PushRuleEvaluator &evaluator) {
          mtx::events::RoomEvent<mtx::events::msg::Text> textEv{};
          textEv.content.body = "abc def ghi honk @room jkl";
          textEv.room_id      = "!abc:def.ghi";
          textEv.event_id     = "$abc1234567890:def.ghi";
          textEv.sender       = "@me:def.ghi";

          mtx::events::state::PowerLevels pls;
          pls.notifications["room"] = 1;
          pls.users["@me:def.ghi"]  = 1;
          mtx::pushrules::PushRuleEvaluator::RoomContext ctx{
            .user_display_name = "me",
            .member_count      = 100,
            .power_levels      = pls,
          };

          EXPECT_EQ(evaluator.evaluate({textEv}, ctx, {}), actions);

          ctx.power_levels.users["@me:def.ghi"] = 0;
          EXPECT_TRUE(evaluator.evaluate({textEv}, ctx, {}).empty());
      };

    mtx::pushrules::Ruleset override_ruleset;
    override_ruleset.override_.push_back(event_match_rule);
    mtx::pushrules::PushRuleEvaluator over_evaluator{override_ruleset};
    testEval(over_evaluator);

    mtx::pushrules::Ruleset underride_ruleset;
    underride_ruleset.underride.push_back(event_match_rule);
    mtx::pushrules::PushRuleEvaluator under_evaluator{underride_ruleset};
    testEval(under_evaluator);
}

TEST(Pushrules, MemberCountMatches)
{
    auto testEval = [](const std::string &is, bool lt, bool eq, bool gt) {
        mtx::events::RoomEvent<mtx::events::msg::Text> textEv{};
        textEv.content.body = "abc def ghi honk @room jkl";
        textEv.room_id      = "!abc:def.ghi";
        textEv.event_id     = "$abc1234567890:def.ghi";
        textEv.sender       = "@me:def.ghi";

        mtx::pushrules::PushRule event_match_rule;
        event_match_rule.actions = {
          mtx::pushrules::actions::notify{},
          mtx::pushrules::actions::set_tweak_highlight{},
        };
        event_match_rule.conditions = {
          mtx::pushrules::PushCondition{
            .kind    = "room_member_count",
            .key     = "",
            .pattern = "",
            .is      = is,
          },
        };
        mtx::pushrules::Ruleset ruleset;
        ruleset.override_.push_back(event_match_rule);
        mtx::pushrules::PushRuleEvaluator evaluator{ruleset};

        mtx::pushrules::PushRuleEvaluator::RoomContext ctx{};

        ctx.member_count = 99;
        EXPECT_EQ(!evaluator.evaluate({textEv}, ctx, {}).empty(), lt);
        ctx.member_count = 100;
        EXPECT_EQ(!evaluator.evaluate({textEv}, ctx, {}).empty(), eq);
        ctx.member_count = 101;
        EXPECT_EQ(!evaluator.evaluate({textEv}, ctx, {}).empty(), gt);
    };

    testEval("100", false, true, false);
    testEval("==100", false, true, false);
    testEval(">=100", false, true, true);
    testEval("<=100", true, true, false);
    testEval(">100", false, false, true);
    testEval("<100", true, false, false);
}

TEST(Pushrules, ContentOverRoomRulesMatches)
{
    json raw_rule = R"(
{
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
        "pattern": "lordmzte",
        "rule_id": ".m.rules.contains_user_name"
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
            "pattern": "@lordmzte:mzte.de"
          }
        ],
        "default": true,
        "enabled": true,
        "rule_id": ".m.rule.invite_for_me"
      },
      {
        "actions": [
          "dont_notify"
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
            "set_tweak": "highlight"
          }
        ],
        "conditions": [
          {
            "key": "type",
            "kind": "event_match",
            "pattern": "m.room.tombstone"
          },
          {
            "key": "state_key",
            "kind": "event_match",
            "pattern": ""
          }
        ],
        "default": true,
        "enabled": false,
        "rule_id": ".m.rule.tombstone"
      },
      {
        "actions": [
          "notify",
          {
            "set_tweak": "highlight"
          }
        ],
        "conditions": [
          {
            "key": "content.body",
            "kind": "event_match",
            "pattern": "@room"
          },
          {
            "key": "room",
            "kind": "sender_notification_permission"
          }
        ],
        "default": true,
        "enabled": true,
        "rule_id": ".m.rule.roomnotif"
      },
      {
        "actions": [
          "dont_notify"
        ],
        "conditions": [
          {
            "key": "type",
            "kind": "event_match",
            "pattern": "m.reaction"
          }
        ],
        "default": true,
        "enabled": true,
        "rule_id": ".m.rule.reaction"
      }
    ],
    "room": [
      {
        "actions": [
          "dont_notify"
        ],
        "default": false,
        "enabled": true,
        "rule_id": "!UbCmIlGTHNIgIRZcpt:nheko.im"
      },
      {
        "actions": [
          "dont_notify"
        ],
        "default": false,
        "enabled": true,
        "rule_id": "!gXyPibMbVgafeoHKIc:matrix.org"
      },
      {
        "actions": [
          "dont_notify"
        ],
        "default": false,
        "enabled": true,
        "rule_id": "!tSZeOIVRJwMOVgkcCT:libera.chat"
      }
    ],
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
        "rule_id": ".m.rules.call"
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
            "is": "2",
            "kind": "room_member_count"
          },
          {
            "key": "type",
            "kind": "event_match",
            "pattern": "m.room.encrypted"
          }
        ],
        "default": true,
        "enabled": true,
        "rule_id": ".m.rules.encrypted_room_one_to_one"
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
            "is": "2",
            "kind": "room_member_count"
          },
          {
            "key": "type",
            "kind": "event_match",
            "pattern": "m.room.message"
          }
        ],
        "default": true,
        "enabled": true,
        "rule_id": ".m.rules.room_one_to_one"
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
        "rule_id": ".m.rules.message"
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
            "pattern": "m.room.encrypted"
          }
        ],
        "default": true,
        "enabled": true,
        "rule_id": ".m.rules.encrypted"
      }
    ]
  }
})"_json;

    mtx::pushrules::GlobalRuleset ruleset = raw_rule.get<mtx::pushrules::GlobalRuleset>();

    json raw_event = R"(
{
    "content": {
        "body": "> <@lordmzte:mzte.de> btw, im still not getting notification on replies, only explicit mentions.\n\nprobably because of your username :p",
        "format": "org.matrix.custom.html",
        "formatted_body": "<mx-reply><blockquote><a href=\"https://matrix.to/#/!UbCmIlGTHNIgIRZcpt:nheko.im/$ifmL9zdEQjnec3LlgxX0Bqr7xVm0agynZBglt7q59AU\">In reply to</a> <a href=\"https://matrix.to/#/@lordmzte:mzte.de\">@lordmzte:mzte.de</a><br/>btw, im still not getting notification on replies, only explicit mentions.</blockquote></mx-reply>probably because of your username :p",
        "im.nheko.relations.v1.relations": [
            {
                "event_id": "$ifmL9zdEQjnec3LlgxX0Bqr7xVm0agynZBglt7q59AU",
                "rel_type": "im.nheko.relations.v1.in_reply_to"
            }
        ],
        "m.relates_to": {
            "m.in_reply_to": {
                "event_id": "$ifmL9zdEQjnec3LlgxX0Bqr7xVm0agynZBglt7q59AU"
            }
        },
        "msgtype": "m.text"
    },
    "event_id": "$mol6Bt546FLBNM7WgTj8mGTAieS8ZsX3JLZ2MH9qQwg",
    "origin_server_ts": 1666006933326,
    "room_id": "!UbCmIlGTHNIgIRZcpt:nheko.im",
    "sender": "@deepbluev7:neko.dev",
    "type": "m.room.message",
    "unsigned": {
        "age": 568918
    }
})"_json;
    auto text      = raw_event.get<mtx::events::RoomEvent<mtx::events::msg::Text>>();
    mtx::pushrules::PushRuleEvaluator evaluator{ruleset.global};
    mtx::pushrules::PushRuleEvaluator::RoomContext ctx{};

    auto actions = evaluator.evaluate({text}, ctx, {});

    auto notifies = [](const std::vector<mtx::pushrules::actions::Action> &a) {
        for (const auto &action : a) {
            if (action == mtx::pushrules::actions::Action{mtx::pushrules::actions::notify{}}) {
                return true;
            }
        }
        return false;
    };

    EXPECT_TRUE(notifies(actions));
}

TEST(Pushrules, ReactionDoesNotMatch)
{
    json raw_rule = R"(
{
  "global": {
    "content": [
        {
            "actions": [
                "notify",
                {
                    "set_tweak": "highlight",
                    "value": false
                }
            ],
            "pattern": "nheko",
            "rule_id": "nheko"
        },
        {
            "actions": [
                "notify",
                {
                    "set_tweak": "highlight",
                    "value": false
                }
            ],
            "pattern": "Nheko",
            "rule_id": "Nheko"
        },
        {
            "actions": [
                "notify",
                {
                    "set_tweak": "highlight",
                    "value": false
                }
            ],
            "pattern": "nico",
            "rule_id": "nico"
        },
        {
            "actions": [
                "notify",
                {
                    "set_tweak": "highlight",
                    "value": false
                }
            ],
            "pattern": "Nico",
            "rule_id": "Nico"
        },
        {
            "actions": [
                "notify",
                {
                    "set_tweak": "highlight",
                    "value": false
                }
            ],
            "pattern": "cc2",
            "rule_id": "cc2"
        }
    ],
    "override": [
        {
            "actions": [
                "dont_notify"
            ],
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
            "rule_id": ".m.rule.suppress_notices"
        },
        {
            "actions": [
                "notify",
                {
                    "set_tweak": "highlight",
                    "value": false
                },
                {
                    "set_tweak": "sound",
                    "value": "default"
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
                    "pattern": "@deepbluev7:neko.dev"
                }
            ],
            "default": true,
            "rule_id": ".m.rule.invite_for_me"
        },
        {
            "actions": [
                "dont_notify"
            ],
            "conditions": [
                {
                    "key": "type",
                    "kind": "event_match",
                    "pattern": "m.room.member"
                }
            ],
            "default": true,
            "rule_id": ".m.rule.member_event"
        },
        {
            "actions": [
                "notify",
                {
                    "set_tweak": "highlight"
                },
                {
                    "set_tweak": "sound",
                    "value": "default"
                }
            ],
            "conditions": [
                {
                    "key": "sender",
                    "kind": "im.nheko.msc3664.related_event_match",
                    "pattern": "@deepbluev7:neko.dev",
                    "rel_type": "m.in_reply_to"
                }
            ],
            "default": true,
            "rule_id": ".im.nheko.msc3664.reply"
        },
        {
            "actions": [
                "notify",
                {
                    "set_tweak": "highlight"
                },
                {
                    "set_tweak": "sound",
                    "value": "default"
                }
            ],
            "conditions": [
                {
                    "kind": "contains_display_name"
                }
            ],
            "default": true,
            "rule_id": ".m.rule.contains_display_name"
        },
        {
            "actions": [
                "notify",
                {
                    "set_tweak": "highlight"
                }
            ],
            "conditions": [
                {
                    "key": "room",
                    "kind": "sender_notification_permission"
                },
                {
                    "key": "content.body",
                    "kind": "event_match",
                    "pattern": "@room"
                }
            ],
            "default": true,
            "rule_id": ".m.rule.roomnotif"
        },
        {
            "actions": [
                "notify",
                {
                    "set_tweak": "highlight"
                }
            ],
            "conditions": [
                {
                    "key": "type",
                    "kind": "event_match",
                    "pattern": "m.room.tombstone"
                },
                {
                    "key": "state_key",
                    "kind": "event_match"
                }
            ],
            "default": true,
            "rule_id": ".m.rule.tombstone"
        },
        {
            "actions": [
                "dont_notify"
            ],
            "conditions": [
                {
                    "key": "type",
                    "kind": "event_match",
                    "pattern": "m.reaction"
                }
            ],
            "default": true,
            "rule_id": ".m.rule.reaction"
        },
        {
            "conditions": [
                {
                    "key": "type",
                    "kind": "event_match",
                    "pattern": "m.room.server_acl"
                },
                {
                    "key": "state_key",
                    "kind": "event_match"
                }
            ],
            "default": true,
            "rule_id": ".m.rule.room.server_acl"
        }
    ],
    "room": [
        {
            "actions": [
                "notify",
                {
                    "set_tweak": "highlight",
                    "value": false
                },
                {
                    "set_tweak": "sound",
                    "value": "default"
                }
            ],
            "rule_id": "!gHDcupDIWdXmlHOhoN:neko.dev"
        }
    ],
    "sender": [
    ],
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
                    "set_tweak": "highlight",
                    "value": false
                }
            ],
            "conditions": [
                {
                    "key": "type",
                    "kind": "event_match",
                    "pattern": "m.room.message"
                },
                {
                    "is": "2",
                    "kind": "room_member_count"
                }
            ],
            "default": true,
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
                    "pattern": "m.room.encrypted"
                },
                {
                    "is": "2",
                    "kind": "room_member_count"
                }
            ],
            "default": true,
            "rule_id": ".m.rule.encrypted_room_one_to_one"
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
                    "pattern": "m.room.message"
                }
            ],
            "default": true,
            "rule_id": ".m.rule.message"
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
                    "pattern": "m.room.encrypted"
                }
            ],
            "default": true,
            "rule_id": ".m.rule.encrypted"
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
                    "pattern": "im.vector.modular.widgets"
                },
                {
                    "key": "content.type",
                    "kind": "event_match",
                    "pattern": "jitsi"
                },
                {
                    "key": "state_key",
                    "kind": "event_match",
                    "pattern": "*"
                }
            ],
            "default": true,
            "rule_id": ".im.vector.jitsi"
        }
    ]
}
})"_json;

    mtx::pushrules::GlobalRuleset ruleset = raw_rule.get<mtx::pushrules::GlobalRuleset>();

    json raw_event = R"(
{
    "content": {
        "m.relates_to": {
            "event_id": "$ifmL9zdEQjnec3LlgxX0Bqr7xVm0agynZBglt7q59AU",
            "key": "a",
            "rel_type": "m.annotation"
        }
    },
    "event_id": "$mol6Bt546FLBNM7WgTj8mGTAieS8ZsX3JLZ2MH9qQwg",
    "origin_server_ts": 1666006933326,
    "room_id": "!UbCmIlGTHNIgIRZcpt:nheko.im",
    "sender": "@deepbluev7:neko.dev",
    "type": "m.reaction",
    "unsigned": {
        "age": 568918
    }
})"_json;
    auto text      = raw_event.get<mtx::events::RoomEvent<mtx::events::msg::Reaction>>();
    mtx::pushrules::PushRuleEvaluator evaluator{ruleset.global};
    mtx::pushrules::PushRuleEvaluator::RoomContext ctx{};

    auto actions = evaluator.evaluate({text}, ctx, {});

    auto notifies = [](const std::vector<mtx::pushrules::actions::Action> &a) {
        for (const auto &action : a) {
            if (action == mtx::pushrules::actions::Action{mtx::pushrules::actions::notify{}}) {
                return true;
            }
        }
        return false;
    };

    EXPECT_FALSE(notifies(actions));
}

TEST(Pushrules, NormalMessageDoesNotHighlight)
{
    json raw_rule =
      R"({
  "global": {
    "underride": [
      {
        "conditions": [
          {
            "kind": "event_match",
            "key": "type",
            "pattern": "m.call.invite"
          }
        ],
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
        "rule_id": ".m.rule.call",
        "default": true,
        "enabled": true
      },
      {
        "conditions": [
          {
            "kind": "event_match",
            "key": "type",
            "pattern": "m.room.message"
          },
          {
            "kind": "room_member_count",
            "is": "2"
          }
        ],
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
        "rule_id": ".m.rule.room_one_to_one",
        "default": true,
        "enabled": true
      },
      {
        "conditions": [
          {
            "kind": "event_match",
            "key": "type",
            "pattern": "m.room.encrypted"
          },
          {
            "kind": "room_member_count",
            "is": "2"
          }
        ],
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
        "rule_id": ".m.rule.encrypted_room_one_to_one",
        "default": true,
        "enabled": true
      },
      {
        "conditions": [
          {
            "kind": "event_match",
            "key": "type",
            "pattern": "m.room.message"
          }
        ],
        "actions": [
          "notify",
          {
            "set_tweak": "highlight",
            "value": false
          }
        ],
        "rule_id": ".m.rule.message",
        "default": true,
        "enabled": true
      },
      {
        "conditions": [
          {
            "kind": "event_match",
            "key": "type",
            "pattern": "m.room.encrypted"
          }
        ],
        "actions": [
          "notify",
          {
            "set_tweak": "highlight",
            "value": false
          }
        ],
        "rule_id": ".m.rule.encrypted",
        "default": true,
        "enabled": true
      },
      {
        "conditions": [
          {
            "kind": "event_match",
            "key": "type",
            "pattern": "im.vector.modular.widgets"
          },
          {
            "kind": "event_match",
            "key": "content.type",
            "pattern": "jitsi"
          },
          {
            "kind": "event_match",
            "key": "state_key",
            "pattern": "*"
          }
        ],
        "actions": [
          "notify",
          {
            "set_tweak": "highlight",
            "value": false
          }
        ],
        "rule_id": ".im.vector.jitsi",
        "default": true,
        "enabled": true
      }
    ],
    "sender": [],
    "room": [
    ],
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
        "pattern": "Stuart",
        "rule_id": "Stuart",
        "default": false,
        "enabled": true
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
        "pattern": "stuart",
        "rule_id": "stuart",
        "default": false,
        "enabled": true
      }
    ],
    "override": [
      {
        "conditions": [],
        "actions": [
          "dont_notify"
        ],
        "rule_id": ".m.rule.master",
        "default": true,
        "enabled": false
      },
      {
        "conditions": [
          {
            "kind": "event_match",
            "key": "content.msgtype",
            "pattern": "m.notice"
          }
        ],
        "actions": [
          "dont_notify"
        ],
        "rule_id": ".m.rule.suppress_notices",
        "default": true,
        "enabled": true
      },
      {
        "conditions": [
          {
            "kind": "event_match",
            "key": "type",
            "pattern": "m.room.member"
          },
          {
            "kind": "event_match",
            "key": "content.membership",
            "pattern": "invite"
          },
          {
            "kind": "event_match",
            "key": "state_key",
            "pattern": "@cadair:cadair.com"
          }
        ],
        "actions": [
          "notify",
          {
            "set_tweak": "highlight",
            "value": false
          },
          {
            "set_tweak": "sound",
            "value": "default"
          }
        ],
        "rule_id": ".m.rule.invite_for_me",
        "default": true,
        "enabled": true
      },
      {
        "conditions": [
          {
            "kind": "event_match",
            "key": "type",
            "pattern": "m.room.member"
          }
        ],
        "actions": [
          "dont_notify"
        ],
        "rule_id": ".m.rule.member_event",
        "default": true,
        "enabled": true
      },
      {
        "conditions": [
          {
            "kind": "contains_display_name"
          }
        ],
        "actions": [
          "notify",
          {
            "set_tweak": "highlight"
          },
          {
            "set_tweak": "sound",
            "value": "default"
          }
        ],
        "rule_id": ".m.rule.contains_display_name",
        "default": true,
        "enabled": true
      },
      {
        "conditions": [
          {
            "kind": "sender_notification_permission",
            "key": "room"
          },
          {
            "kind": "event_match",
            "key": "content.body",
            "pattern": "@room"
          }
        ],
        "actions": [
          "notify",
          {
            "set_tweak": "highlight"
          }
        ],
        "rule_id": ".m.rule.roomnotif",
        "default": true,
        "enabled": true
      },
      {
        "conditions": [
          {
            "kind": "event_match",
            "key": "type",
            "pattern": "m.room.tombstone"
          },
          {
            "kind": "event_match",
            "key": "state_key",
            "pattern": ""
          }
        ],
        "actions": [
          "notify",
          {
            "set_tweak": "highlight"
          }
        ],
        "rule_id": ".m.rule.tombstone",
        "default": true,
        "enabled": true
      },
      {
        "conditions": [
          {
            "kind": "event_match",
            "key": "type",
            "pattern": "m.reaction"
          }
        ],
        "actions": [
          "dont_notify"
        ],
        "rule_id": ".m.rule.reaction",
        "default": true,
        "enabled": true
      },
      {
        "conditions": [
          {
            "kind": "event_match",
            "key": "type",
            "pattern": "m.room.server_acl"
          },
          {
            "kind": "event_match",
            "key": "state_key",
            "pattern": ""
          }
        ],
        "actions": [],
        "rule_id": ".m.rule.room.server_acl",
        "default": true,
        "enabled": true
      }
    ]
  },
  "device": {}
}
)"_json;

    mtx::pushrules::GlobalRuleset ruleset = raw_rule.get<mtx::pushrules::GlobalRuleset>();

    json raw_event = R"({
    "content": {
        "body": "You can hide them in the room settings",
        "msgtype": "m.text"
    },
    "event_id": "$4gnZ8Vop7BnXjpri2qH4wk2xZ7PRWzqMJWhDboe8Upc",
    "origin_server_ts": 1667410048371,
    "sender": "@deepbluev7:neko.dev",
    "type": "m.room.message",
    "unsigned": {
        "age": 69,
        "transaction_id": "mEfzSS17er68gvakLESSodiRsoDwvBqdk"
    }
}
)"_json;
    auto text      = raw_event.get<mtx::events::RoomEvent<mtx::events::msg::Text>>();
    mtx::pushrules::PushRuleEvaluator evaluator{ruleset.global};
    mtx::pushrules::PushRuleEvaluator::RoomContext ctx{};

    auto actions = evaluator.evaluate({text}, ctx, {});

    auto notifies = [](const std::vector<mtx::pushrules::actions::Action> &a) {
        for (const auto &action : a) {
            if (action == mtx::pushrules::actions::Action{mtx::pushrules::actions::notify{}}) {
                return true;
            }
        }
        return false;
    };

    EXPECT_TRUE(notifies(actions));
    for (const auto &action : actions) {
        EXPECT_NE(action,
                  mtx::pushrules::actions::Action{mtx::pushrules::actions::set_tweak_highlight{}});
    }
}

TEST(Pushrules, RelatedEventMatch)
{
    json raw_rule = R"(
{
  "global": {
    "content": [
    ],
    "override": [
        {
            "actions": [
                "dont_notify"
            ],
            "default": true,
            "enabled": false,
            "rule_id": ".m.rule.master"
        },
        {
            "actions": [
                "notify",
                {
                    "set_tweak": "highlight"
                },
                {
                    "set_tweak": "sound",
                    "value": "default"
                }
            ],
            "conditions": [
                {
                    "key": "sender",
                    "kind": "im.nheko.msc3664.related_event_match",
                    "pattern": "@deepbluev7:neko.dev",
                    "rel_type": "m.in_reply_to"
                }
            ],
            "default": true,
            "rule_id": ".im.nheko.msc3664.reply"
        }
    ],
    "room": [
    ],
    "sender": [
    ],
    "underride": [
    ]
}
})"_json;

    mtx::pushrules::GlobalRuleset ruleset = raw_rule.get<mtx::pushrules::GlobalRuleset>();

    json raw_event         = R"(
{
    "content": {
        "body": "You can hide them in the room settings",
        "msgtype": "m.text",
        "m.relates_to": {
            "m.in_reply_to": {
              "event_id": "$aaaaaaaaaaaaaaaaaaaa:bc.de"
            },
            "event_id": "$aaaaaaaaaaaaaaaaaaaa:bc.de",
            "rel_type": "m.thread"
        }
    },
    "event_id": "$mol6Bt546FLBNM7WgTj8mGTAieS8ZsX3JLZ2MH9qQwg",
    "origin_server_ts": 1666006933326,
    "room_id": "!UbCmIlGTHNIgIRZcpt:nheko.im",
    "sender": "@nico:neko.dev",
    "type": "m.room.message"
})"_json;
    json related_raw_event = R"(
{
    "content": {
        "body": "You can hide them in the room settings",
        "msgtype": "m.text"
    },
    "event_id": "$aaaaaaaaaaaaaaaaaaaa:bc.de",
    "origin_server_ts": 1666006933326,
    "room_id": "!UbCmIlGTHNIgIRZcpt:nheko.im",
    "sender": "@deepbluev7:neko.dev",
    "type": "m.room.message"
})"_json;

    auto text         = raw_event.get<mtx::events::RoomEvent<mtx::events::msg::Text>>();
    auto related_text = related_raw_event.get<mtx::events::RoomEvent<mtx::events::msg::Text>>();
    mtx::pushrules::PushRuleEvaluator evaluator{ruleset.global};
    mtx::pushrules::PushRuleEvaluator::RoomContext ctx{};

    auto actions = evaluator.evaluate({text},
                                      ctx,
                                      {
                                        {text.content.relations.relations.at(0), {related_text}},
                                        {text.content.relations.relations.at(1), {related_text}},
                                      });

    auto notifies = [](const std::vector<mtx::pushrules::actions::Action> &a) {
        for (const auto &action : a) {
            if (action == mtx::pushrules::actions::Action{mtx::pushrules::actions::notify{}}) {
                return true;
            }
        }
        return false;
    };

    EXPECT_TRUE(
      notifies(evaluator.evaluate({text},
                                  ctx,
                                  {
                                    {text.content.relations.relations.at(0), {related_text}},
                                    {text.content.relations.relations.at(1), {related_text}},
                                  })));
    EXPECT_FALSE(notifies(evaluator.evaluate({text}, ctx, {})));

    raw_rule = R"(
{
  "global": {
    "content": [
    ],
    "override": [
        {
            "actions": [
                "dont_notify"
            ],
            "default": true,
            "enabled": false,
            "rule_id": ".m.rule.master"
        },
        {
            "actions": [
                "notify",
                {
                    "set_tweak": "highlight"
                },
                {
                    "set_tweak": "sound",
                    "value": "default"
                }
            ],
            "conditions": [
                {
                    "key": "sender",
                    "kind": "im.nheko.msc3664.related_event_match",
                    "pattern": "@deepbluev7:neko.dev",
                    "rel_type": "m.thread"
                }
            ],
            "default": true,
            "rule_id": ".im.nheko.msc3664.reply"
        }
    ],
    "room": [
    ],
    "sender": [
    ],
    "underride": [
    ]
}
})"_json;

    ruleset = raw_rule.get<mtx::pushrules::GlobalRuleset>();
    mtx::pushrules::PushRuleEvaluator evaluator1{ruleset.global};
    EXPECT_TRUE(
      notifies(evaluator1.evaluate({text},
                                   ctx,
                                   {
                                     {text.content.relations.relations.at(0), {related_text}},
                                     {text.content.relations.relations.at(1), {related_text}},
                                   })));
    EXPECT_FALSE(notifies(evaluator1.evaluate({text}, ctx, {})));

    raw_rule = R"(
{
  "global": {
    "content": [
    ],
    "override": [
        {
            "actions": [
                "dont_notify"
            ],
            "default": true,
            "enabled": false,
            "rule_id": ".m.rule.master"
        },
        {
            "actions": [
                "notify",
                {
                    "set_tweak": "highlight"
                },
                {
                    "set_tweak": "sound",
                    "value": "default"
                }
            ],
            "conditions": [
                {
                    "kind": "im.nheko.msc3664.related_event_match",
                    "rel_type": "m.thread"
                }
            ],
            "default": true,
            "rule_id": ".im.nheko.msc3664.reply"
        }
    ],
    "room": [
    ],
    "sender": [
    ],
    "underride": [
    ]
}
})"_json;

    ruleset = raw_rule.get<mtx::pushrules::GlobalRuleset>();
    mtx::pushrules::PushRuleEvaluator evaluator2{ruleset.global};
    EXPECT_TRUE(
      notifies(evaluator2.evaluate({text},
                                   ctx,
                                   {
                                     {text.content.relations.relations.at(0), {related_text}},
                                     {text.content.relations.relations.at(1), {related_text}},
                                   })));
    EXPECT_FALSE(notifies(evaluator2.evaluate({text}, ctx, {})));

    raw_rule = R"(
{
  "global": {
    "content": [
    ],
    "override": [
        {
            "actions": [
                "dont_notify"
            ],
            "default": true,
            "enabled": false,
            "rule_id": ".m.rule.master"
        },
        {
            "actions": [
                "notify",
                {
                    "set_tweak": "highlight"
                },
                {
                    "set_tweak": "sound",
                    "value": "default"
                }
            ],
            "conditions": [
                {
                    "key": "sender",
                    "kind": "im.nheko.msc3664.related_event_match",
                    "pattern": "@deepbluev8:neko.dev",
                    "rel_type": "m.in_reply_to"
                }
            ],
            "default": true,
            "rule_id": ".im.nheko.msc3664.reply"
        }
    ],
    "room": [
    ],
    "sender": [
    ],
    "underride": [
    ]
}
})"_json;

    ruleset = raw_rule.get<mtx::pushrules::GlobalRuleset>();
    mtx::pushrules::PushRuleEvaluator evaluator3{ruleset.global};
    EXPECT_FALSE(
      notifies(evaluator3.evaluate({text},
                                   ctx,
                                   {
                                     {text.content.relations.relations.at(0), {related_text}},
                                     {text.content.relations.relations.at(1), {related_text}},
                                   })));
    EXPECT_FALSE(notifies(evaluator3.evaluate({text}, ctx, {})));

    raw_event = R"(
{
    "content": {
        "body": "You can hide them in the room settings",
        "msgtype": "m.text",
        "m.relates_to": {
            "m.in_reply_to": {
              "event_id": "$aaaaaaaaaaaaaaaaaaaa:bc.de"
            },
            "is_falling_back": true,
            "event_id": "$aaaaaaaaaaaaaaaaaaaa:bc.de",
            "rel_type": "m.thread"
        }
    },
    "event_id": "$mol6Bt546FLBNM7WgTj8mGTAieS8ZsX3JLZ2MH9qQwg",
    "origin_server_ts": 1666006933326,
    "room_id": "!UbCmIlGTHNIgIRZcpt:nheko.im",
    "sender": "@nico:neko.dev",
    "type": "m.room.message"
})"_json;
    text      = raw_event.get<mtx::events::RoomEvent<mtx::events::msg::Text>>();
    EXPECT_FALSE(
      notifies(evaluator.evaluate({text},
                                  ctx,
                                  {
                                    {text.content.relations.relations.at(0), {related_text}},
                                    {text.content.relations.relations.at(1), {related_text}},
                                  })));
    EXPECT_FALSE(notifies(evaluator.evaluate({text}, ctx, {})));

    raw_rule = R"(
{
  "global": {
    "content": [
    ],
    "override": [
        {
            "actions": [
                "dont_notify"
            ],
            "default": true,
            "enabled": false,
            "rule_id": ".m.rule.master"
        },
        {
            "actions": [
                "notify",
                {
                    "set_tweak": "highlight"
                },
                {
                    "set_tweak": "sound",
                    "value": "default"
                }
            ],
            "conditions": [
                {
                    "key": "sender",
                    "kind": "im.nheko.msc3664.related_event_match",
                    "pattern": "@deepbluev7:neko.dev",
                    "rel_type": "m.in_reply_to",
                    "include_fallback": true
                }
            ],
            "default": true,
            "rule_id": ".im.nheko.msc3664.reply"
        }
    ],
    "room": [
    ],
    "sender": [
    ],
    "underride": [
    ]
}
})"_json;

    ruleset = raw_rule.get<mtx::pushrules::GlobalRuleset>();
    mtx::pushrules::PushRuleEvaluator evaluator4{ruleset.global};
    EXPECT_TRUE(
      notifies(evaluator4.evaluate({text},
                                   ctx,
                                   {
                                     {text.content.relations.relations.at(0), {related_text}},
                                     {text.content.relations.relations.at(1), {related_text}},
                                   })));
    EXPECT_FALSE(notifies(evaluator4.evaluate({text}, ctx, {})));

    raw_rule = R"(
{
  "global": {
    "content": [
    ],
    "override": [
        {
            "actions": [
                "dont_notify"
            ],
            "default": true,
            "enabled": false,
            "rule_id": ".m.rule.master"
        },
        {
            "actions": [
                "notify",
                {
                    "set_tweak": "highlight"
                },
                {
                    "set_tweak": "sound",
                    "value": "default"
                }
            ],
            "conditions": [
                {
                    "key": "sender",
                    "kind": "im.nheko.msc3664.related_event_match",
                    "pattern": "@deepbluev7:neko.dev",
                    "rel_type": "m.in_reply_to"
                }
            ],
            "default": true,
            "rule_id": ".im.nheko.msc3664.reply"
        }
    ],
    "room": [
    ],
    "sender": [
    ],
    "underride": [
    ]
}
})"_json;

    raw_event  = R"(
{
    "content": {
        "body": "You can hide them in the room settings",
        "msgtype": "m.emote",
        "m.relates_to": {
            "m.in_reply_to": {
              "event_id": "$aaaaaaaaaaaaaaaaaaaa:bc.de"
            },
            "event_id": "$aaaaaaaaaaaaaaaaaaaa:bc.de",
            "rel_type": "m.thread"
        }
    },
    "event_id": "$mol6Bt546FLBNM7WgTj8mGTAieS8ZsX3JLZ2MH9qQwg",
    "origin_server_ts": 1666006933326,
    "room_id": "!UbCmIlGTHNIgIRZcpt:nheko.im",
    "sender": "@nico:neko.dev",
    "type": "m.room.message"
})"_json;
    auto emote = raw_event.get<mtx::events::RoomEvent<mtx::events::msg::Emote>>();

    ruleset = raw_rule.get<mtx::pushrules::GlobalRuleset>();
    mtx::pushrules::PushRuleEvaluator evaluator5{ruleset.global};
    EXPECT_TRUE(
      notifies(evaluator5.evaluate({emote},
                                   ctx,
                                   {
                                     {emote.content.relations.relations.at(0), {related_text}},
                                     {emote.content.relations.relations.at(1), {related_text}},
                                   })));
    EXPECT_FALSE(notifies(evaluator5.evaluate({emote}, ctx, {})));
}

TEST(Pushrules, EmptyOrInvalidContentRulesDontMatch)
{
    json raw_rule = R"(
{
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
                        "set_tweak": "highlight",
                        "value": true
                    }
                ],
                "conditions": [
                    {
                        "kind": "contains_user_mxid"
                    }
                ],
                "default": true,
                "enabled": true,
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
                        "kind": "state_key_user_mxid"
                    }
                ],
                "default": true,
                "enabled": true,
                "rule_id": ".m.rule.invite_for_me"
            },
            {
                "actions": [
                    "dont_notify"
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
                        "set_tweak": "highlight",
                        "value": true
                    }
                ],
                "conditions": [
                    {
                        "key": "type",
                        "kind": "event_match",
                        "pattern": "m.room.tombstone"
                    },
                    {
                        "key": "state_key",
                        "kind": "event_match",
                        "pattern": ""
                    }
                ],
                "default": true,
                "enabled": true,
                "rule_id": ".m.rule.tombstone"
            },
            {
                "actions": [
                    "notify",
                    {
                        "set_tweak": "highlight",
                        "value": true
                    }
                ],
                "conditions": [
                    {
                        "key": "content.body",
                        "kind": "event_match",
                        "pattern": "@room"
                    },
                    {
                        "key": "room",
                        "kind": "sender_notification_permission"
                    }
                ],
                "default": true,
                "enabled": true,
                "rule_id": ".m.rule.roomnotif"
            },
            {
                "actions": [
                    "dont_notify"
                ],
                "conditions": [
                    {
                        "key": "type",
                        "kind": "event_match",
                        "pattern": "m.reaction"
                    }
                ],
                "default": true,
                "enabled": true,
                "rule_id": ".m.rule.reaction"
            },
            {
                "actions": [
                    "dont_notify"
                ],
                "conditions": [
                    {
                        "key": "room_id",
                        "kind": "event_match",
                        "pattern": "!eMqocHMcvmMFuUpioO:matrix.org"
                    }
                ],
                "default": false,
                "enabled": true,
                "rule_id": "!eMqocHMcvmMFuUpioO:matrix.org"
            },
            {
                "actions": [
                    "dont_notify"
                ],
                "conditions": [
                    {
                        "key": "room_id",
                        "kind": "event_match",
                        "pattern": "!n6gxWBq2KfcWGcQ5IV:zemos.net"
                    }
                ],
                "default": false,
                "enabled": true,
                "rule_id": "!n6gxWBq2KfcWGcQ5IV:zemos.net"
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
                        "set_tweak": "highlight",
                        "value": false
                    }
                ],
                "conditions": [
                    {
                        "key": "type",
                        "kind": "event_match",
                        "pattern": "m.room.encrypted"
                    },
                    {
                        "is": "2",
                        "kind": "room_member_count"
                    }
                ],
                "default": true,
                "enabled": true,
                "rule_id": ".m.rule.encrypted_room_one_to_one"
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
                        "pattern": "m.room.message"
                    },
                    {
                        "is": "2",
                        "kind": "room_member_count"
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
                        "set_tweak": "highlight",
                        "value": false
                    }
                ],
                "conditions": [
                    {
                        "key": "type",
                        "kind": "event_match",
                        "pattern": "m.room.encrypted"
                    }
                ],
                "default": true,
                "enabled": true,
                "rule_id": ".m.rule.encrypted"
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
}
 )"_json;

    mtx::pushrules::GlobalRuleset ruleset = raw_rule.get<mtx::pushrules::GlobalRuleset>();

    json raw_event = R"(
{
    "content": {
        "body": "> <@lordmzte:mzte.de> btw, im still not getting notification on replies, only explicit mentions.\n\nprobably because of your username :p",
        "format": "org.matrix.custom.html",
        "formatted_body": "<mx-reply><blockquote><a href=\"https://matrix.to/#/!UbCmIlGTHNIgIRZcpt:nheko.im/$ifmL9zdEQjnec3LlgxX0Bqr7xVm0agynZBglt7q59AU\">In reply to</a> <a href=\"https://matrix.to/#/@lordmzte:mzte.de\">@lordmzte:mzte.de</a><br/>btw, im still not getting notification on replies, only explicit mentions.</blockquote></mx-reply>probably because of your username :p",
        "im.nheko.relations.v1.relations": [
            {
                "event_id": "$ifmL9zdEQjnec3LlgxX0Bqr7xVm0agynZBglt7q59AU",
                "rel_type": "im.nheko.relations.v1.in_reply_to"
            }
        ],
        "m.relates_to": {
            "m.in_reply_to": {
                "event_id": "$ifmL9zdEQjnec3LlgxX0Bqr7xVm0agynZBglt7q59AU"
            }
        },
        "msgtype": "m.text"
    },
    "event_id": "$mol6Bt546FLBNM7WgTj8mGTAieS8ZsX3JLZ2MH9qQwg",
    "origin_server_ts": 1666006933326,
    "room_id": "!UbCmIlGTHNIgIRZcpt:nheko.im",
    "sender": "@deepbluev7:neko.dev",
    "type": "m.room.message",
    "unsigned": {
        "age": 568918
    }
})"_json;
    auto text      = raw_event.get<mtx::events::RoomEvent<mtx::events::msg::Text>>();
    mtx::pushrules::PushRuleEvaluator evaluator{ruleset.global};
    mtx::pushrules::PushRuleEvaluator::RoomContext ctx{};
    ctx.user_display_name = "Jason";

    auto actions = evaluator.evaluate({text}, ctx, {});

    auto notifies = [](const std::vector<mtx::pushrules::actions::Action> &a) {
        for (const auto &action : a) {
            if (action ==
                mtx::pushrules::actions::Action{mtx::pushrules::actions::set_tweak_highlight{}}) {
                return true;
            }
        }
        return false;
    };

    EXPECT_FALSE(notifies(actions));
}
