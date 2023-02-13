#include <gtest/gtest.h>

#include <mtx.hpp>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

namespace ns = mtx::events;

TEST(Events, Redaction)
{
    json data = R"({
	  "unsigned": {
            "age": 146
	  },
          "content": {
              "reason": "No reason"
          },
          "event_id": "$143273582443PhrSn:localhost",
          "origin_server_ts": 1432735824653,
          "room_id": "!jEsUZKDJdhlrceRyVU:localhost",
          "redacts": "$1521361675759563UDexf:matrix.org",
          "sender": "@example:localhost",
          "type": "m.room.redaction"
        })"_json;

    ns::RedactionEvent<ns::msg::Redaction> event =
      data.get<ns::RedactionEvent<ns::msg::Redaction>>();

    EXPECT_EQ(event.type, ns::EventType::RoomRedaction);
    EXPECT_EQ(event.event_id, "$143273582443PhrSn:localhost");
    EXPECT_EQ(event.room_id, "!jEsUZKDJdhlrceRyVU:localhost");
    EXPECT_EQ(event.sender, "@example:localhost");
    EXPECT_EQ(event.origin_server_ts, 1432735824653L);
    EXPECT_EQ(event.unsigned_data.age, 146);
    EXPECT_EQ(event.redacts, "$1521361675759563UDexf:matrix.org");

    EXPECT_EQ(event.content.reason, "No reason");
}

TEST(Events, Redacted)
{
    json data = R"({
	  "unsigned": {
            "age": 146
	  },
          "content": {
          },
          "event_id": "$143273582443PhrSn:localhost",
          "origin_server_ts": 1432735824653,
          "room_id": "!jEsUZKDJdhlrceRyVU:localhost",
          "redacts": "$1521361675759563UDexf:matrix.org",
          "sender": "@example:localhost",
          "type": "m.room.redaction"
        })"_json;

    mtx::events::collections::TimelineEvent event =
      data.get<mtx::events::collections::TimelineEvent>();

    ASSERT_TRUE(std::holds_alternative<ns::RedactionEvent<ns::msg::Redaction>>(event.data));

    json data2 = R"({
		"content": {
			"membership": "join"
		},
		"origin_server_ts": 1595256121167,
		"sender": "@redacted_user_1:example.com",
		"state_key": "@redacted_user_1:example.com",
		"type": "m.room.member",
		"unsigned": {
			"redacted_by": "$redacted_id_1",
			"redacted_because": {
				"content": {},
				"origin_server_ts": 1595261803914,
				"redacts": "$redacted_id_2",
				"sender": "@redacted_user_2:example.com",
				"type": "m.room.redaction",
				"unsigned": {
					"age": 23764322
				},
				"event_id": "$redacted_id_1"
			}
		},
		"event_id": "$redacted_id_2"
	})"_json;

    event = data2.get<mtx::events::collections::TimelineEvent>();
    ASSERT_TRUE(std::holds_alternative<ns::StateEvent<ns::msg::Redacted>>(event.data));
    ASSERT_TRUE(std::get<ns::StateEvent<ns::msg::Redacted>>(event.data)
                  .unsigned_data.redacted_because.has_value());
    ASSERT_EQ(std::get<ns::StateEvent<ns::msg::Redacted>>(event.data)
                .unsigned_data.redacted_because->sender,
              "@redacted_user_2:example.com");
}

TEST(Events, Conversions)
{
    EXPECT_EQ("m.room.aliases", ns::to_string(ns::EventType::RoomAliases));
    EXPECT_EQ("m.room.avatar", ns::to_string(ns::EventType::RoomAvatar));
    EXPECT_EQ("m.room.canonical_alias", ns::to_string(ns::EventType::RoomCanonicalAlias));
    EXPECT_EQ("m.room.create", ns::to_string(ns::EventType::RoomCreate));
    EXPECT_EQ("m.room.guest_access", ns::to_string(ns::EventType::RoomGuestAccess));
    EXPECT_EQ("m.room.history_visibility", ns::to_string(ns::EventType::RoomHistoryVisibility));
    EXPECT_EQ("m.room.join_rules", ns::to_string(ns::EventType::RoomJoinRules));
    EXPECT_EQ("m.room.member", ns::to_string(ns::EventType::RoomMember));
    EXPECT_EQ("m.room.message", ns::to_string(ns::EventType::RoomMessage));
    EXPECT_EQ("m.room.name", ns::to_string(ns::EventType::RoomName));
    EXPECT_EQ("m.room.power_levels", ns::to_string(ns::EventType::RoomPowerLevels));
    EXPECT_EQ("m.room.topic", ns::to_string(ns::EventType::RoomTopic));
    EXPECT_EQ("m.room.tombstone", ns::to_string(ns::EventType::RoomTombstone));
    EXPECT_EQ("m.room.redaction", ns::to_string(ns::EventType::RoomRedaction));
    EXPECT_EQ("m.room.pinned_events", ns::to_string(ns::EventType::RoomPinnedEvents));
    EXPECT_EQ("m.policy.rule.user", ns::to_string(ns::EventType::PolicyRuleUser));
    EXPECT_EQ("m.policy.rule.room", ns::to_string(ns::EventType::PolicyRuleRoom));
    EXPECT_EQ("m.policy.rule.server", ns::to_string(ns::EventType::PolicyRuleServer));
    EXPECT_EQ("m.space.child", ns::to_string(ns::EventType::SpaceChild));
    EXPECT_EQ("m.space.parent", ns::to_string(ns::EventType::SpaceParent));
    EXPECT_EQ("m.tag", ns::to_string(ns::EventType::Tag));
}

TEST(StateEvents, Aliases)
{
    json data = R"({
	  "unsigned": {
	    "age": 242352,
	    "transaction_id": "txnid"
	  },
	  "content": {
	    "aliases": [
	      "#somewhere:localhost",
	      "#another:localhost"
	    ]
	  },
	  "event_id": "$WLGTSEFSEF:localhost",
	  "origin_server_ts": 1431961217939,
          "room_id": "!Cuyf34gef24t:localhost",
	  "sender": "@example:localhost",
	  "state_key": "localhost",
	  "type": "m.room.aliases"
	})"_json;

    ns::StateEvent<ns::state::Aliases> aliases = data.get<ns::StateEvent<ns::state::Aliases>>();

    EXPECT_EQ(aliases.type, ns::EventType::RoomAliases);
    EXPECT_EQ(aliases.event_id, "$WLGTSEFSEF:localhost");
    EXPECT_EQ(aliases.room_id, "!Cuyf34gef24t:localhost");
    EXPECT_EQ(aliases.sender, "@example:localhost");
    EXPECT_EQ(aliases.unsigned_data.age, 242352);
    EXPECT_EQ(aliases.unsigned_data.transaction_id, "txnid");
    EXPECT_EQ(aliases.origin_server_ts, 1431961217939L);
    EXPECT_EQ(aliases.state_key, "localhost");
    EXPECT_EQ(aliases.content.aliases.size(), 2);
    EXPECT_EQ(aliases.content.aliases[0], "#somewhere:localhost");
    EXPECT_EQ(aliases.content.aliases[1], "#another:localhost");
}

TEST(StateEvents, Avatar)
{
    json data = R"({
          "origin_server_ts": 1506762071625,
          "sender": "@mujx:matrix.org",
          "event_id": "$15067620711415511reUFC:matrix.org",
          "age": 3717700323,
          "unsigned": {
            "age": 3717700323
          },
          "state_key": "",
          "content": {
            "url": "mxc://matrix.org/EPKcIsAPzREMrvZIjrIuJnEl"
          },
          "room_id": "!VaMCVKSVcyPtXbcMXh:matrix.org",
          "user_id": "@mujx:matrix.org",
          "type": "m.room.avatar"
        })"_json;

    ns::StateEvent<ns::state::Avatar> event = data.get<ns::StateEvent<ns::state::Avatar>>();

    EXPECT_EQ(event.type, ns::EventType::RoomAvatar);
    EXPECT_EQ(event.event_id, "$15067620711415511reUFC:matrix.org");
    EXPECT_EQ(event.room_id, "!VaMCVKSVcyPtXbcMXh:matrix.org");
    EXPECT_EQ(event.sender, "@mujx:matrix.org");
    EXPECT_EQ(event.unsigned_data.age, 3717700323);
    EXPECT_EQ(event.origin_server_ts, 1506762071625L);
    EXPECT_EQ(event.state_key, "");
    EXPECT_EQ(event.content.url, "mxc://matrix.org/EPKcIsAPzREMrvZIjrIuJnEl");
}

TEST(StateEvents, CanonicalAlias)
{
    json data = R"({
    "content": {
        "alias": "#somewhere:localhost",
        "alt_aliases": [
            "#somewhere:example.org",
            "#myroom:example.com"
        ]
    },
    "event_id": "$143273582443PhrSn:example.org",
    "origin_server_ts": 1432735824653,
    "room_id": "!jEsUZKDJdhlrceRyVU:example.org",
    "sender": "@example:example.org",
    "state_key": "",
    "type": "m.room.canonical_alias",
    "unsigned": {
        "age": 1234
    }
})"_json;

    ns::StateEvent<ns::state::CanonicalAlias> event =
      data.get<ns::StateEvent<ns::state::CanonicalAlias>>();

    EXPECT_EQ(event.type, ns::EventType::RoomCanonicalAlias);
    EXPECT_EQ(event.event_id, "$143273582443PhrSn:example.org");
    EXPECT_EQ(event.room_id, "!jEsUZKDJdhlrceRyVU:example.org");
    EXPECT_EQ(event.sender, "@example:example.org");
    EXPECT_EQ(event.unsigned_data.age, 1234);
    EXPECT_EQ(event.origin_server_ts, 1432735824653L);
    EXPECT_EQ(event.state_key, "");
    EXPECT_EQ(event.content.alias, "#somewhere:localhost");
    ASSERT_EQ(event.content.alt_aliases.size(), 2);
    EXPECT_EQ(event.content.alt_aliases.at(0), "#somewhere:example.org");
    EXPECT_EQ(event.content.alt_aliases.at(1), "#myroom:example.com");
}

TEST(StateEvents, Create)
{
    json data = R"({
          "origin_server_ts": 1506761923948,
          "sender": "@mujx:matrix.org",
          "event_id": "$15067619231414398jhvQC:matrix.org",
          "unsigned": {
            "age": 3715756343
          },
          "state_key": "",
          "content": {
            "creator": "@mujx:matrix.org"
          },
          "type": "m.room.create"
        })"_json;

    ns::StateEvent<ns::state::Create> event = data.get<ns::StateEvent<ns::state::Create>>();

    EXPECT_EQ(event.type, ns::EventType::RoomCreate);
    EXPECT_EQ(event.event_id, "$15067619231414398jhvQC:matrix.org");
    EXPECT_EQ(event.sender, "@mujx:matrix.org");
    EXPECT_EQ(event.unsigned_data.age, 3715756343L);
    EXPECT_EQ(event.origin_server_ts, 1506761923948L);
    EXPECT_EQ(event.state_key, "");
    EXPECT_EQ(event.content.creator, "@mujx:matrix.org");

    json example_from_spec = R"({
            "content": {
                "creator": "@example:example.org",
                "m.federate": true,
                "predecessor": {
                    "event_id": "$something:example.org",
                    "room_id": "!oldroom:example.org"
                },
                "room_version": "1"
            },
            "event_id": "$143273582443PhrSn:example.org",
            "origin_server_ts": 1432735824653,
            "room_id": "!jEsUZKDJdhlrceRyVU:example.org",
            "sender": "@example:example.org",
            "state_key": "",
            "type": "m.room.create",
            "unsigned": {
                "age": 1234
            }
        })"_json;

    event = example_from_spec.get<ns::StateEvent<ns::state::Create>>();

    EXPECT_EQ(event.type, ns::EventType::RoomCreate);
    EXPECT_EQ(event.event_id, "$143273582443PhrSn:example.org");
    EXPECT_EQ(event.sender, "@example:example.org");
    EXPECT_EQ(event.unsigned_data.age, 1234);
    EXPECT_EQ(event.origin_server_ts, 1432735824653L);
    EXPECT_EQ(event.state_key, "");
    EXPECT_EQ(event.content.creator, "@example:example.org");
    EXPECT_EQ(event.content.federate, true);
    EXPECT_EQ(event.content.room_version, "1");
    EXPECT_EQ(event.content.predecessor->room_id, "!oldroom:example.org");
    EXPECT_EQ(event.content.predecessor->event_id, "$something:example.org");
}

TEST(StateEvents, CreateWithType)
{
    json data = R"({
          "origin_server_ts": 1506761923948,
          "sender": "@mujx:matrix.org",
          "event_id": "$15067619231414398jhvQC:matrix.org",
          "unsigned": {
            "age": 3715756343
          },
          "state_key": "",
          "content": {
            "creator": "@mujx:matrix.org",
            "type": "m.space"
          },
          "type": "m.room.create"
        })"_json;

    ns::StateEvent<ns::state::Create> event = data.get<ns::StateEvent<ns::state::Create>>();

    EXPECT_EQ(event.type, ns::EventType::RoomCreate);
    EXPECT_EQ(event.event_id, "$15067619231414398jhvQC:matrix.org");
    EXPECT_EQ(event.sender, "@mujx:matrix.org");
    EXPECT_EQ(event.unsigned_data.age, 3715756343L);
    EXPECT_EQ(event.origin_server_ts, 1506761923948L);
    EXPECT_EQ(event.state_key, "");
    EXPECT_EQ(event.content.creator, "@mujx:matrix.org");
    EXPECT_TRUE(event.content.type.has_value());
    EXPECT_EQ(event.content.type.value(), ns::state::room_type::space);

    json example_from_spec = R"({
  "content": {
    "creator": "@deepbluev7:neko.dev",
    "room_version": "6",
    "type": "m.space"
  },
  "origin_server_ts": 1623788764437,
  "sender": "@deepbluev7:neko.dev",
  "state_key": "",
  "type": "m.room.create",
  "unsigned": {
    "age": 2241800
  },
  "event_id": "$VXf-Ze1j8D3KQ95b66sB7cNp1LzCqOHOJ8nk2iHtZvE",
  "room_id": "!KLPDfgYGHhdZWLbUwD:neko.dev"
})"_json;

    event = example_from_spec.get<ns::StateEvent<ns::state::Create>>();

    EXPECT_TRUE(event.content.type.has_value());
    EXPECT_EQ(event.content.type.value(), ns::state::room_type::space);

    json room_v10 = R"(
{
  "content": {
    "creator": "@test:neko.dev",
    "predecessor": {
      "event_id": "$hpB30pm_PDHq9dcz7zJR6gwnEFRPxzea7J8bxVuuSTg",
      "room_id": "!isYaZDvAOnwGmoNKgD:neko.dev"
    },
    "room_version": "10"
  },
  "origin_server_ts": 1670903493618,
  "sender": "@test:neko.dev",
  "state_key": "",
  "type": "m.room.create",
  "unsigned": {
    "age": 118
  },
  "event_id": "$2NH68IAcuHuAwsy1WEyLqAnOH_iAVjyUu2GP5IksnYc",
  "room_id": "!FhwxBVrewHNqikGaXN:neko.dev"
}
    )"_json;

    event = room_v10.get<ns::StateEvent<ns::state::Create>>();

    EXPECT_EQ(event.content.room_version, "10");
}

TEST(StateEvents, GuestAccess)
{
    json data = R"({
          "origin_server_ts": 1506761923948,
          "sender": "@mujx:matrix.org",
          "event_id": "$15067619231414398jhvQC:matrix.org",
          "unsigned": {
            "age": 3715756343
          },
          "state_key": "",
          "content": {
            "guest_access": "can_join"
          },
          "type": "m.room.guest_access"
        })"_json;

    ns::StateEvent<ns::state::GuestAccess> event =
      data.get<ns::StateEvent<ns::state::GuestAccess>>();

    EXPECT_EQ(event.type, ns::EventType::RoomGuestAccess);
    EXPECT_EQ(event.event_id, "$15067619231414398jhvQC:matrix.org");
    EXPECT_EQ(event.sender, "@mujx:matrix.org");
    EXPECT_EQ(event.unsigned_data.age, 3715756343L);
    EXPECT_EQ(event.origin_server_ts, 1506761923948L);
    EXPECT_EQ(event.state_key, "");
    EXPECT_EQ(event.content.guest_access, mtx::events::state::AccessState::CanJoin);
}

TEST(StateEvents, HistoryVisibility)
{
    json data = R"({
	  "origin_server_ts": 1510473133072,
	  "sender": "@nheko_test:matrix.org",
	  "event_id": "$15104731332646268uOFJp:matrix.org",
	  "unsigned": {
	    "age": 140
	  },
	  "state_key": "",
  	  "content": {
	    "history_visibility": "shared"
	  },
	  "type": "m.room.history_visibility",
	  "room_id": "!lfoDRlNFWlvOnvkBwQ:matrix.org"
	})"_json;

    ns::StateEvent<ns::state::HistoryVisibility> event =
      data.get<ns::StateEvent<ns::state::HistoryVisibility>>();

    EXPECT_EQ(event.type, ns::EventType::RoomHistoryVisibility);
    EXPECT_EQ(event.event_id, "$15104731332646268uOFJp:matrix.org");
    EXPECT_EQ(event.room_id, "!lfoDRlNFWlvOnvkBwQ:matrix.org");
    EXPECT_EQ(event.sender, "@nheko_test:matrix.org");
    EXPECT_EQ(event.origin_server_ts, 1510473133072L);
    EXPECT_EQ(event.unsigned_data.age, 140);
    EXPECT_EQ(event.state_key, "");
    EXPECT_EQ(event.content.history_visibility, ns::state::Visibility::Shared);

    json data2 = R"({
          "origin_server_ts": 1510476778190,
          "sender": "@nheko_test:matrix.org",
          "event_id": "$15104767782674661tXoeB:matrix.org",
          "unsigned": {
            "prev_content": {
              "history_visibility": "shared"
            },
            "prev_sender": "@nheko_test:matrix.org",
            "replaces_state": "$15104731332646268uOFJp:matrix.org",
            "age": 48
          },
          "state_key": "",
          "content": {
            "history_visibility": "invited"
          },
          "type": "m.room.history_visibility",
          "room_id": "!lfoDRlNFWlvOnvkBwQ:matrix.org"
        })"_json;

    ns::StateEvent<ns::state::HistoryVisibility> event2 =
      data2.get<ns::StateEvent<ns::state::HistoryVisibility>>();

    EXPECT_EQ(event2.type, ns::EventType::RoomHistoryVisibility);
    EXPECT_EQ(event2.event_id, "$15104767782674661tXoeB:matrix.org");
    EXPECT_EQ(event2.room_id, "!lfoDRlNFWlvOnvkBwQ:matrix.org");
    EXPECT_EQ(event2.sender, "@nheko_test:matrix.org");
    EXPECT_EQ(event2.origin_server_ts, 1510476778190L);
    EXPECT_EQ(event2.unsigned_data.age, 48);
    EXPECT_EQ(event2.unsigned_data.replaces_state, "$15104731332646268uOFJp:matrix.org");
    EXPECT_EQ(event2.state_key, "");
    EXPECT_EQ(event2.content.history_visibility, ns::state::Visibility::Invited);
}

TEST(StateEvents, JoinRules)
{
    json data = R"({
          "origin_server_ts": 1506761924018,
          "sender": "@mujx:matrix.org",
          "event_id": "$15067619241414401ASocy:matrix.org",
          "unsigned": {
            "age": 3715756273
	  },
          "state_key": "",
          "content": {
            "join_rule": "invite"
	  },
          "type": "m.room.join_rules"
        })"_json;

    ns::StateEvent<ns::state::JoinRules> event = data.get<ns::StateEvent<ns::state::JoinRules>>();

    EXPECT_EQ(event.type, ns::EventType::RoomJoinRules);
    EXPECT_EQ(event.event_id, "$15067619241414401ASocy:matrix.org");
    EXPECT_EQ(event.sender, "@mujx:matrix.org");
    EXPECT_EQ(event.unsigned_data.age, 3715756273);
    EXPECT_EQ(event.origin_server_ts, 1506761924018L);
    EXPECT_EQ(event.state_key, "");
    EXPECT_EQ(event.content.join_rule, ns::state::JoinRule::Invite);

    EXPECT_EQ(data, json(event));

    data = R"({
          "origin_server_ts": 1506761924018,
          "sender": "@mujx:matrix.org",
          "event_id": "$15067619241414401ASocy:matrix.org",
          "unsigned": {
            "age": 3715756273
	  },
          "state_key": "",
          "content": {
            "join_rule": "public"
	  },
          "type": "m.room.join_rules"
        })"_json;

    EXPECT_EQ(data, json(ns::StateEvent<ns::state::JoinRules>(data)));

    data = R"({
          "origin_server_ts": 1506761924018,
          "sender": "@mujx:matrix.org",
          "event_id": "$15067619241414401ASocy:matrix.org",
          "unsigned": {
            "age": 3715756273
	  },
          "state_key": "",
          "content": {
            "join_rule": "knock"
	  },
          "type": "m.room.join_rules"
        })"_json;

    EXPECT_EQ(data, json(ns::StateEvent<ns::state::JoinRules>(data)));

    data = R"({
          "origin_server_ts": 1506761924018,
          "sender": "@mujx:matrix.org",
          "event_id": "$15067619241414401ASocy:matrix.org",
          "unsigned": {
            "age": 3715756273
	  },
          "state_key": "",
          "content": {
            "join_rule": "private"
	  },
          "type": "m.room.join_rules"
        })"_json;

    EXPECT_EQ(data, json(ns::StateEvent<ns::state::JoinRules>(data)));

    data = R"({
          "type": "m.room.join_rules",
          "state_key": "",
          "sender": "@mujx:matrix.org",
          "event_id": "$15067619241414401ASocy:matrix.org",
          "origin_server_ts": 1506761924018,
          "content": {
              "join_rule": "restricted",
              "allow": [
                  {
                      "type": "m.room_membership",
                      "room_id": "!mods:example.org"
                  },
                  {
                      "type": "m.room_membership",
                      "room_id": "!users:example.org"
                  }
              ]
          }
      })"_json;

    ns::StateEvent<ns::state::JoinRules> event2 = data.get<ns::StateEvent<ns::state::JoinRules>>();
    ASSERT_EQ(event2.content.allow.size(), 2);
    ASSERT_EQ(event2.content.allow[0].type, mtx::events::state::JoinAllowanceType::RoomMembership);
    ASSERT_EQ(event2.content.allow[0].room_id, "!mods:example.org");
    ASSERT_EQ(event2.content.allow[1].type, mtx::events::state::JoinAllowanceType::RoomMembership);
    ASSERT_EQ(event2.content.allow[1].room_id, "!users:example.org");
}

TEST(StateEvents, Member)
{
    json data = R"({
          "origin_server_ts": 1510473132947,
          "sender": "@nheko_test:matrix.org",
          "event_id": "$15104731322646264oUPqj:matrix.org",
          "unsigned": {
            "age": 72
          },
          "state_key": "@nheko_test:matrix.org",
          "content": {
            "membership": "join",
            "avatar_url": "mxc://matrix.org/JKiSOBDDxCHxmaLAgoQwSAHa",
            "displayname": "NhekoTest"
          },
          "membership": "join",
          "type": "m.room.member",
          "room_id": "!lfoDRlNFWlvOnvkBwQ:matrix.org"
	})"_json;

    ns::StateEvent<ns::state::Member> event = data.get<ns::StateEvent<ns::state::Member>>();

    EXPECT_EQ(event.type, ns::EventType::RoomMember);
    EXPECT_EQ(event.event_id, "$15104731322646264oUPqj:matrix.org");
    EXPECT_EQ(event.room_id, "!lfoDRlNFWlvOnvkBwQ:matrix.org");
    EXPECT_EQ(event.sender, "@nheko_test:matrix.org");
    EXPECT_EQ(event.origin_server_ts, 1510473132947L);
    EXPECT_EQ(event.unsigned_data.age, 72);
    EXPECT_EQ(event.state_key, "@nheko_test:matrix.org");
    EXPECT_EQ(event.content.membership, ns::state::Membership::Join);
    EXPECT_EQ(event.content.display_name, "NhekoTest");
    EXPECT_EQ(event.content.avatar_url, "mxc://matrix.org/JKiSOBDDxCHxmaLAgoQwSAHa");

    json data2 = R"({
          "prev_content": {
            "membership": "join",
            "avatar_url": "mxc://matrix.org/IvqcnGakfvwwKeZxbJWhblFp",
            "displayname": "NhekoTest"
          },
          "origin_server_ts": 1509214100884,
          "sender": "@nheko_test:matrix.org",
          "event_id": "$15092141005099019aHvYG:matrix.org",
          "age": 1259000688,
          "unsigned": {
            "prev_content": {
              "membership": "join",
              "avatar_url": "mxc://matrix.org/IvqcnGakfvwwKeZxbJWhblFp",
              "displayname": "NhekoTest"
            },
            "prev_sender": "@nheko_test:matrix.org",
            "replaces_state": "$15092124385075888YpYOh:matrix.org",
            "age": 1259000688
          },
          "state_key": "@nheko_test:matrix.org",
          "content": {
            "membership": "join",
            "avatar_url": "mxc://matrix.org/JKiSOBDDxCHxmaLAgoQwSAHa",
            "displayname": "NhekoTest"
          },
          "membership": "join",
          "room_id": "!VaMCVKSVcyPtXbcMXh:matrix.org",
          "user_id": "@nheko_test:matrix.org",
          "replaces_state": "$15092124385075888YpYOh:matrix.org",
          "type": "m.room.member"
        })"_json;

    ns::StateEvent<ns::state::Member> event2 = data2.get<ns::StateEvent<ns::state::Member>>();

    EXPECT_EQ(event2.type, ns::EventType::RoomMember);
    EXPECT_EQ(event2.event_id, "$15092141005099019aHvYG:matrix.org");
    EXPECT_EQ(event2.room_id, "!VaMCVKSVcyPtXbcMXh:matrix.org");
    EXPECT_EQ(event2.sender, "@nheko_test:matrix.org");
    EXPECT_EQ(event2.origin_server_ts, 1509214100884L);
    EXPECT_EQ(event2.unsigned_data.replaces_state, "$15092124385075888YpYOh:matrix.org");
    EXPECT_EQ(event2.unsigned_data.age, 1259000688);
    EXPECT_EQ(event2.unsigned_data.age, 1259000688);
    EXPECT_EQ(event2.state_key, "@nheko_test:matrix.org");
    EXPECT_EQ(event2.content.membership, ns::state::Membership::Join);
    EXPECT_EQ(event2.content.display_name, "NhekoTest");
    EXPECT_EQ(event2.content.avatar_url, "mxc://matrix.org/JKiSOBDDxCHxmaLAgoQwSAHa");
}

TEST(StateEvents, Name)
{
    json data = R"({
          "origin_server_ts": 1510473133142,
          "sender": "@nheko_test:matrix.org",
          "event_id": "$15104731332646270uaKBS:matrix.org",
          "unsigned": {
            "age": 70
          },
          "state_key": "",
          "content": {
            "name": "Random name"
          },
          "type": "m.room.name",
          "room_id": "!lfoDRlNFWlvOnvkBwQ:matrix.org"
        })"_json;

    ns::StateEvent<ns::state::Name> event = data.get<ns::StateEvent<ns::state::Name>>();

    EXPECT_EQ(event.type, ns::EventType::RoomName);
    EXPECT_EQ(event.event_id, "$15104731332646270uaKBS:matrix.org");
    EXPECT_EQ(event.room_id, "!lfoDRlNFWlvOnvkBwQ:matrix.org");
    EXPECT_EQ(event.sender, "@nheko_test:matrix.org");
    EXPECT_EQ(event.origin_server_ts, 1510473133142L);
    EXPECT_EQ(event.unsigned_data.age, 70);
    EXPECT_EQ(event.state_key, "");
    EXPECT_EQ(event.content.name, "Random name");
}

TEST(StateEvents, PinnedEvents)
{
    json data = R"({
	  "unsigned": {
            "age": 242352
	  },
          "content": {
            "pinned": [
              "$one:localhost",
              "$two:localhost",
              "$three:localhost"
            ]
          },
          "event_id": "$WLGTSEFSEF:localhost",
          "origin_server_ts": 1431961217939,
          "sender": "@example:localhost",
          "state_key": "",
          "type": "m.room.pinned_events"
        })"_json;

    ns::StateEvent<ns::state::PinnedEvents> event =
      data.get<ns::StateEvent<ns::state::PinnedEvents>>();

    EXPECT_EQ(event.type, ns::EventType::RoomPinnedEvents);
    EXPECT_EQ(event.event_id, "$WLGTSEFSEF:localhost");
    EXPECT_EQ(event.sender, "@example:localhost");
    EXPECT_EQ(event.origin_server_ts, 1431961217939L);
    EXPECT_EQ(event.unsigned_data.age, 242352);
    EXPECT_EQ(event.state_key, "");

    EXPECT_EQ(event.content.pinned.size(), 3);
    EXPECT_EQ(event.content.pinned[0], "$one:localhost");
    EXPECT_EQ(event.content.pinned[1], "$two:localhost");
    EXPECT_EQ(event.content.pinned[2], "$three:localhost");
}

TEST(StateEvents, PowerLevels)
{
    json data = R"({
          "origin_server_ts": 1506761923995,
          "sender": "@mujx:matrix.org",
          "event_id": "$15067619231414400iQDgf:matrix.org",
          "unsigned": {
            "age": 3715756296
	  },
          "state_key": "",
          "content": {
            "events_default": 0,
            "invite": 0,
            "state_default": 50,
            "redact": 50,
            "ban": 50,
            "users_default": 0,
            "kick": 50,
            "events": {
              "m.room.avatar": 29,
              "m.room.name": 50,
              "m.room.canonical_alias":	50,
              "m.room.history_visibility": 120,
              "m.room.power_levels": 109
	    },
            "users": {
              "@mujx:matrix.org": 30
	    }
	  },
          "type": "m.room.power_levels"
	})"_json;

    ns::StateEvent<ns::state::PowerLevels> event =
      data.get<ns::StateEvent<ns::state::PowerLevels>>();

    EXPECT_EQ(event.type, ns::EventType::RoomPowerLevels);
    EXPECT_EQ(event.event_id, "$15067619231414400iQDgf:matrix.org");
    EXPECT_EQ(event.sender, "@mujx:matrix.org");
    EXPECT_EQ(event.origin_server_ts, 1506761923995);
    EXPECT_EQ(event.unsigned_data.age, 3715756296);
    EXPECT_EQ(event.state_key, "");

    EXPECT_EQ(event.content.kick, 50);
    EXPECT_EQ(event.content.ban, 50);
    EXPECT_EQ(event.content.invite, 0);
    EXPECT_EQ(event.content.redact, 50);
    EXPECT_EQ(event.content.events_default, 0);
    EXPECT_EQ(event.content.users_default, 0);
    EXPECT_EQ(event.content.state_default, 50);

    EXPECT_EQ(event.content.events.size(), 5);
    EXPECT_EQ(event.content.events["m.room.name"], 50);
    EXPECT_EQ(event.content.events["m.room.avatar"], 29);
    EXPECT_EQ(event.content.events["m.room.canonical_alias"], 50);
    EXPECT_EQ(event.content.events["m.room.history_visibility"], 120);
    EXPECT_EQ(event.content.events["m.room.power_levels"], 109);

    EXPECT_EQ(event.content.users.size(), 1);
    EXPECT_EQ(event.content.users["@mujx:matrix.org"], 30);

    EXPECT_EQ(event.content.event_level("m.room.name"), 50);
    EXPECT_EQ(event.content.event_level("m.room.avatar"), 29);
    EXPECT_EQ(event.content.event_level("m.room.canonical_alias"), 50);
    EXPECT_EQ(event.content.event_level("m.room.history_visibility"), 120);
    EXPECT_EQ(event.content.event_level("m.room.power_levels"), 109);
    EXPECT_EQ(event.content.event_level("m.custom.event"), event.content.events_default);

    EXPECT_EQ(event.content.user_level("@mujx:matrix.org"), 30);
    EXPECT_EQ(event.content.user_level("@not:matrix.org"), event.content.users_default);
}

TEST(StateEvents, Tombstone)
{
    json data = R"({
            "content": {
                "body": "This room has been replaced",
                "replacement_room": "!newroom:example.org"
            },
            "event_id": "$143273582443PhrSn:example.org",
            "origin_server_ts": 1432735824653,
            "room_id": "!jEsUZKDJdhlrceRyVU:example.org",
            "sender": "@example:example.org",
            "state_key": "",
            "type": "m.room.tombstone",
            "unsigned": {
                "age": 1234
            }
        })"_json;

    ns::StateEvent<ns::state::Tombstone> event = data.get<ns::StateEvent<ns::state::Tombstone>>();

    EXPECT_EQ(event.type, ns::EventType::RoomTombstone);
    EXPECT_EQ(event.event_id, "$143273582443PhrSn:example.org");
    EXPECT_EQ(event.room_id, "!jEsUZKDJdhlrceRyVU:example.org");
    EXPECT_EQ(event.sender, "@example:example.org");
    EXPECT_EQ(event.origin_server_ts, 1432735824653);
    EXPECT_EQ(event.unsigned_data.age, 1234);
    EXPECT_EQ(event.state_key, "");
    EXPECT_EQ(event.content.body, "This room has been replaced");
    EXPECT_EQ(event.content.replacement_room, "!newroom:example.org");
}

TEST(StateEvents, Topic)
{
    json data = R"({
          "origin_server_ts": 1510476064445,
          "sender": "@nheko_test:matrix.org",
          "event_id": "$15104760642668662QICBu:matrix.org",
          "unsigned": {
            "age": 37
          },
          "state_key": "",
          "content": {
            "topic": "Test topic"
          },
          "type": "m.room.topic",
          "room_id": "!lfoDRlNFWlvOnvkBwQ:matrix.org"
        })"_json;

    ns::StateEvent<ns::state::Topic> event = data.get<ns::StateEvent<ns::state::Topic>>();

    EXPECT_EQ(event.type, ns::EventType::RoomTopic);
    EXPECT_EQ(event.event_id, "$15104760642668662QICBu:matrix.org");
    EXPECT_EQ(event.room_id, "!lfoDRlNFWlvOnvkBwQ:matrix.org");
    EXPECT_EQ(event.sender, "@nheko_test:matrix.org");
    EXPECT_EQ(event.origin_server_ts, 1510476064445);
    EXPECT_EQ(event.unsigned_data.age, 37);
    EXPECT_EQ(event.state_key, "");
    EXPECT_EQ(event.content.topic, "Test topic");
}

TEST(StateEvents, PolicyRuleUser)
{
    json data = R"(
{
    "content": {
        "entity": "@alice*:example.org",
        "reason": "undesirable behaviour",
        "recommendation": "m.ban"
    },
    "event_id": "$143273582443PhrSn:example.org",
    "origin_server_ts": 1432735824653,
    "room_id": "!jEsUZKDJdhlrceRyVU:example.org",
    "sender": "@example:example.org",
    "state_key": "rule:@alice*:example.org",
    "type": "m.policy.rule.user",
    "unsigned": {
        "age": 1234
    }
}
        )"_json;

    auto event = data.get<ns::StateEvent<ns::state::policy_rule::UserRule>>();

    EXPECT_EQ(event.type, ns::EventType::PolicyRuleUser);
    EXPECT_EQ(event.event_id, "$143273582443PhrSn:example.org");
    EXPECT_EQ(event.content.entity, "@alice*:example.org");
    EXPECT_EQ(event.content.reason, "undesirable behaviour");
    EXPECT_EQ(event.content.recommendation, ns::state::policy_rule::recommendation::ban);

    data  = R"(
{
    "content": null,
    "event_id": "$143273582443PhrSn:example.org",
    "origin_server_ts": 1432735824653,
    "room_id": "!jEsUZKDJdhlrceRyVU:example.org",
    "sender": "@example:example.org",
    "state_key": "rule:@alice*:example.org",
    "type": "m.policy.rule.user",
    "unsigned": {
        "age": 1234
    }
}
        )"_json;
    event = data.get<ns::StateEvent<ns::state::policy_rule::UserRule>>();
    EXPECT_EQ(event.type, ns::EventType::PolicyRuleUser);
}

TEST(StateEvents, PolicyRuleRoom)
{
    json data = R"(
{
    "content": {
        "entity": "#*:example.org",
        "reason": "undesirable content",
        "recommendation": "m.ban"
    },
    "event_id": "$143273582443PhrSn:example.org",
    "origin_server_ts": 1432735824653,
    "room_id": "!jEsUZKDJdhlrceRyVU:example.org",
    "sender": "@example:example.org",
    "state_key": "rule:@alice*:example.org",
    "type": "m.policy.rule.room",
    "unsigned": {
        "age": 1234
    }
}
        )"_json;

    auto event = data.get<ns::StateEvent<ns::state::policy_rule::RoomRule>>();

    EXPECT_EQ(event.type, ns::EventType::PolicyRuleRoom);
    EXPECT_EQ(event.event_id, "$143273582443PhrSn:example.org");
    EXPECT_EQ(event.content.entity, "#*:example.org");
    EXPECT_EQ(event.content.reason, "undesirable content");
    EXPECT_EQ(event.content.recommendation, ns::state::policy_rule::recommendation::ban);
}

TEST(StateEvents, PolicyRuleServer)
{
    json data = R"(
{
    "content": {
        "entity": "*.example.org",
        "reason": "undesirable engagement",
        "recommendation": "m.ban"
    },
    "event_id": "$143273582443PhrSn:example.org",
    "origin_server_ts": 1432735824653,
    "room_id": "!jEsUZKDJdhlrceRyVU:example.org",
    "sender": "@example:example.org",
    "state_key": "rule:@alice*:example.org",
    "type": "m.policy.rule.server",
    "unsigned": {
        "age": 1234
    }
}
        )"_json;

    auto event = data.get<ns::StateEvent<ns::state::policy_rule::ServerRule>>();

    EXPECT_EQ(event.type, ns::EventType::PolicyRuleServer);
    EXPECT_EQ(event.event_id, "$143273582443PhrSn:example.org");
    EXPECT_EQ(event.content.entity, "*.example.org");
    EXPECT_EQ(event.content.reason, "undesirable engagement");
    EXPECT_EQ(event.content.recommendation, ns::state::policy_rule::recommendation::ban);
}

TEST(StateEvents, SpaceChild)
{
    json data = R"({
          "origin_server_ts": 1510476064445,
          "sender": "@nheko_test:matrix.org",
          "event_id": "$15104760642668662QICBu:matrix.org",
      "type": "m.space.child",
      "state_key": "!abcd:example.com",
      "content": {
          "via": ["example.com", "test.org"]
      }
}
        )"_json;

    ns::StateEvent<ns::state::space::Child> event =
      data.get<ns::StateEvent<ns::state::space::Child>>();

    EXPECT_EQ(event.type, ns::EventType::SpaceChild);
    EXPECT_EQ(event.event_id, "$15104760642668662QICBu:matrix.org");
    EXPECT_EQ(event.sender, "@nheko_test:matrix.org");
    EXPECT_EQ(event.origin_server_ts, 1510476064445);
    EXPECT_EQ(event.state_key, "!abcd:example.com");
    ASSERT_TRUE(event.content.via.has_value());
    std::vector<std::string> via{"example.com", "test.org"};
    EXPECT_EQ(event.content.via, via);
    EXPECT_FALSE(event.content.order.has_value());

    data = R"({
          "origin_server_ts": 1510476064445,
          "sender": "@nheko_test:matrix.org",
          "event_id": "$15104760642668662QICBu:matrix.org",
        "type": "m.space.child",
        "state_key": "!efgh:example.com",
        "content": {
        "via": ["example.com"],
        "order": "abcd"
    }
}
        )"_json;

    event = data.get<ns::StateEvent<ns::state::space::Child>>();

    EXPECT_EQ(event.type, ns::EventType::SpaceChild);
    EXPECT_EQ(event.event_id, "$15104760642668662QICBu:matrix.org");
    EXPECT_EQ(event.sender, "@nheko_test:matrix.org");
    EXPECT_EQ(event.origin_server_ts, 1510476064445);
    EXPECT_EQ(event.state_key, "!efgh:example.com");
    ASSERT_TRUE(event.content.via.has_value());
    std::vector<std::string> via2{"example.com"};
    EXPECT_EQ(event.content.via, via2);
    ASSERT_TRUE(event.content.order.has_value());
    ASSERT_EQ(event.content.order, "abcd");

    data = R"({
          "origin_server_ts": 1510476064445,
          "sender": "@nheko_test:matrix.org",
          "event_id": "$15104760642668662QICBu:matrix.org",
        "type": "m.space.child",
        "state_key": "!jklm:example.com",
        "content": {}
}
        )"_json;

    event = data.get<ns::StateEvent<ns::state::space::Child>>();

    EXPECT_EQ(event.type, ns::EventType::SpaceChild);
    EXPECT_EQ(event.event_id, "$15104760642668662QICBu:matrix.org");
    EXPECT_EQ(event.sender, "@nheko_test:matrix.org");
    EXPECT_EQ(event.origin_server_ts, 1510476064445);
    EXPECT_EQ(event.state_key, "!jklm:example.com");
    ASSERT_FALSE(event.content.via.has_value());
    ASSERT_FALSE(event.content.order.has_value());

    data = R"({
          "origin_server_ts": 1510476064445,
          "sender": "@nheko_test:matrix.org",
          "event_id": "$15104760642668662QICBu:matrix.org",
        "type": "m.space.child",
        "state_key": "!efgh:example.com",
        "content": {
        "via": ["example.com"],
        "order": "01234567890123456789012345678901234567890123456789_"
    }
}
        )"_json;

    event = data.get<ns::StateEvent<ns::state::space::Child>>();

    EXPECT_EQ(event.type, ns::EventType::SpaceChild);
    EXPECT_EQ(event.event_id, "$15104760642668662QICBu:matrix.org");
    EXPECT_EQ(event.sender, "@nheko_test:matrix.org");
    EXPECT_EQ(event.origin_server_ts, 1510476064445);
    EXPECT_EQ(event.state_key, "!efgh:example.com");
    EXPECT_TRUE(event.content.via.has_value());
    ASSERT_FALSE(event.content.order.has_value());

    data = R"({
          "origin_server_ts": 1510476064445,
          "sender": "@nheko_test:matrix.org",
          "event_id": "$15104760642668662QICBu:matrix.org",
        "type": "m.space.child",
        "state_key": "!efgh:example.com",
        "content": {
        "via": [],
        "order": "01234567890123456789012345678901234567890123456789_"
    }
}
        )"_json;

    event = data.get<ns::StateEvent<ns::state::space::Child>>();

    EXPECT_FALSE(event.content.via.has_value());

    data = R"({
          "origin_server_ts": 1510476064445,
          "sender": "@nheko_test:matrix.org",
          "event_id": "$15104760642668662QICBu:matrix.org",
        "type": "m.space.child",
        "state_key": "!efgh:example.com",
        "content": {
        "via": 5,
        "order": "01234567890123456789012345678901234567890123456789_"
    }
}
        )"_json;

    event = data.get<ns::StateEvent<ns::state::space::Child>>();

    EXPECT_FALSE(event.content.via.has_value());
    data = R"({
          "origin_server_ts": 1510476064445,
          "sender": "@nheko_test:matrix.org",
          "event_id": "$15104760642668662QICBu:matrix.org",
        "type": "m.space.child",
        "state_key": "!efgh:example.com",
        "content": {
        "via": null,
        "order": "01234567890123456789012345678901234567890123456789_"
    }
}
        )"_json;

    event = data.get<ns::StateEvent<ns::state::space::Child>>();

    EXPECT_FALSE(event.content.via.has_value());

    data = R"({
          "origin_server_ts": 1510476064445,
          "sender": "@nheko_test:matrix.org",
          "event_id": "$15104760642668662QICBu:matrix.org",
        "type": "m.space.child",
        "state_key": "!efgh:example.com",
        "content": {
        "order": "01234567890123456789012345678901234567890123456789_"
    }
}
        )"_json;

    event = data.get<ns::StateEvent<ns::state::space::Child>>();

    EXPECT_FALSE(event.content.via.has_value());
}
TEST(StateEvents, SpaceParent)
{
    json data = R"({
          "origin_server_ts": 1510476064445,
          "sender": "@nheko_test:matrix.org",
          "event_id": "$15104760642668662QICBu:matrix.org",
          "type": "m.space.parent",
          "state_key": "!space:example.com",
          "content": {
            "via": ["example.com"],
            "canonical": true
          }
        })"_json;

    ns::StateEvent<ns::state::space::Parent> event =
      data.get<ns::StateEvent<ns::state::space::Parent>>();

    EXPECT_EQ(event.type, ns::EventType::SpaceParent);
    EXPECT_EQ(event.event_id, "$15104760642668662QICBu:matrix.org");
    EXPECT_EQ(event.sender, "@nheko_test:matrix.org");
    EXPECT_EQ(event.origin_server_ts, 1510476064445);
    EXPECT_EQ(event.state_key, "!space:example.com");
    ASSERT_TRUE(event.content.via.has_value());
    std::vector<std::string> via{"example.com"};
    EXPECT_EQ(event.content.via, via);
    EXPECT_TRUE(event.content.canonical);

    data = R"({
          "origin_server_ts": 1510476064445,
          "sender": "@nheko_test:matrix.org",
          "event_id": "$15104760642668662QICBu:matrix.org",
          "type": "m.space.parent",
          "state_key": "!space:example.com",
          "content": {
            "via": ["example.org"]
          }
        })"_json;

    event = data.get<ns::StateEvent<ns::state::space::Parent>>();

    EXPECT_EQ(event.type, ns::EventType::SpaceParent);
    EXPECT_EQ(event.event_id, "$15104760642668662QICBu:matrix.org");
    EXPECT_EQ(event.sender, "@nheko_test:matrix.org");
    EXPECT_EQ(event.origin_server_ts, 1510476064445);
    EXPECT_EQ(event.state_key, "!space:example.com");
    EXPECT_TRUE(event.content.via.has_value());
    EXPECT_FALSE(event.content.canonical);

    data = R"({
          "origin_server_ts": 1510476064445,
          "sender": "@nheko_test:matrix.org",
          "event_id": "$15104760642668662QICBu:matrix.org",
          "type": "m.space.parent",
          "state_key": "!space:example.com",
          "content": {
            "via": [],
            "canonical": true
          }
        })"_json;

    event = data.get<ns::StateEvent<ns::state::space::Parent>>();

    EXPECT_EQ(event.type, ns::EventType::SpaceParent);
    EXPECT_EQ(event.event_id, "$15104760642668662QICBu:matrix.org");
    EXPECT_EQ(event.sender, "@nheko_test:matrix.org");
    EXPECT_EQ(event.origin_server_ts, 1510476064445);
    EXPECT_EQ(event.state_key, "!space:example.com");
    EXPECT_FALSE(event.content.via.has_value());
    EXPECT_TRUE(event.content.canonical);

    data = R"({
          "origin_server_ts": 1510476064445,
          "sender": "@nheko_test:matrix.org",
          "event_id": "$15104760642668662QICBu:matrix.org",
          "type": "m.space.parent",
          "state_key": "!space:example.com",
          "content": {
            "via": null,
            "canonical": true
          }
        })"_json;

    event = data.get<ns::StateEvent<ns::state::space::Parent>>();

    EXPECT_EQ(event.type, ns::EventType::SpaceParent);
    EXPECT_EQ(event.event_id, "$15104760642668662QICBu:matrix.org");
    EXPECT_EQ(event.sender, "@nheko_test:matrix.org");
    EXPECT_EQ(event.origin_server_ts, 1510476064445);
    EXPECT_EQ(event.state_key, "!space:example.com");
    EXPECT_FALSE(event.content.via.has_value());
    EXPECT_TRUE(event.content.canonical);

    data = R"({
          "origin_server_ts": 1510476064445,
          "sender": "@nheko_test:matrix.org",
          "event_id": "$15104760642668662QICBu:matrix.org",
          "type": "m.space.parent",
          "state_key": "!space:example.com",
          "content": {
            "via": 5,
            "canonical": true
          }
        })"_json;

    event = data.get<ns::StateEvent<ns::state::space::Parent>>();

    EXPECT_EQ(event.type, ns::EventType::SpaceParent);
    EXPECT_EQ(event.event_id, "$15104760642668662QICBu:matrix.org");
    EXPECT_EQ(event.sender, "@nheko_test:matrix.org");
    EXPECT_EQ(event.origin_server_ts, 1510476064445);
    EXPECT_EQ(event.state_key, "!space:example.com");
    EXPECT_FALSE(event.content.via.has_value());
    EXPECT_TRUE(event.content.canonical);
    data = R"({
          "origin_server_ts": 1510476064445,
          "sender": "@nheko_test:matrix.org",
          "event_id": "$15104760642668662QICBu:matrix.org",
          "type": "m.space.parent",
          "state_key": "!space:example.com",
          "content": {
            "via": "adjsa",
            "canonical": true
          }
        })"_json;

    event = data.get<ns::StateEvent<ns::state::space::Parent>>();

    EXPECT_EQ(event.type, ns::EventType::SpaceParent);
    EXPECT_EQ(event.event_id, "$15104760642668662QICBu:matrix.org");
    EXPECT_EQ(event.sender, "@nheko_test:matrix.org");
    EXPECT_EQ(event.origin_server_ts, 1510476064445);
    EXPECT_EQ(event.state_key, "!space:example.com");
    EXPECT_FALSE(event.content.via.has_value());
    EXPECT_TRUE(event.content.canonical);
}

TEST(StateEvents, ImagePack)
{
    json data = R"({
          "origin_server_ts": 1510476064445,
          "sender": "@nheko_test:matrix.org",
          "event_id": "$15104760642668662QICBu:matrix.org",
          "unsigned": {
            "age": 37
          },
          "state_key": "my-pack",
          "content": {
  "images": {
    "emote": {
      "url": "mxc://example.org/blah"
    },
    "sticker": {
      "url": "mxc://example.org/sticker",
      "body": "stcikerly",
      "usage": ["sticker"]
    }
  },
  "pack": {
    "display_name": "Awesome Pack",
    "avatar_url": "mxc://example.org/asdjfasd",
    "usage": ["emoticon"],
    "attribution": "huh"
  }
},
          "type": "im.ponies.room_emotes",
          "room_id": "!lfoDRlNFWlvOnvkBwQ:matrix.org"
        })"_json;

    ns::StateEvent<ns::msc2545::ImagePack> event =
      data.get<ns::StateEvent<ns::msc2545::ImagePack>>();

    EXPECT_EQ(event.type, ns::EventType::ImagePackInRoom);
    EXPECT_EQ(event.event_id, "$15104760642668662QICBu:matrix.org");
    EXPECT_EQ(event.room_id, "!lfoDRlNFWlvOnvkBwQ:matrix.org");
    EXPECT_EQ(event.sender, "@nheko_test:matrix.org");
    EXPECT_EQ(event.origin_server_ts, 1510476064445);
    EXPECT_EQ(event.unsigned_data.age, 37);
    EXPECT_EQ(event.state_key, "my-pack");
    ASSERT_EQ(event.content.pack.has_value(), true);
    assert(event.content.pack);
    EXPECT_EQ(event.content.pack->display_name, "Awesome Pack");
    EXPECT_EQ(event.content.pack->attribution, "huh");
    EXPECT_EQ(event.content.pack->avatar_url, "mxc://example.org/asdjfasd");
    EXPECT_EQ(event.content.pack->is_emoji(), true);
    EXPECT_EQ(event.content.pack->is_sticker(), false);
    ASSERT_EQ(event.content.images.size(), 2);
    EXPECT_EQ(event.content.images["emote"].url, "mxc://example.org/blah");
    EXPECT_EQ(event.content.images["emote"].overrides_usage(), false);
    EXPECT_EQ(event.content.images["sticker"].url, "mxc://example.org/sticker");
    EXPECT_EQ(event.content.images["sticker"].body, "stcikerly");
    EXPECT_EQ(event.content.images["sticker"].is_sticker(), true);
    EXPECT_EQ(event.content.images["sticker"].is_emoji(), false);
    EXPECT_EQ(event.content.images["sticker"].overrides_usage(), true);
    EXPECT_EQ(json(event)["content"]["images"].size(), 2);

    json data2 = R"({
          "origin_server_ts": 1510476064445,
          "sender": "@nheko_test:matrix.org",
          "event_id": "$15104760642668662QICBu:matrix.org",
          "unsigned": {
            "age": 37
          },
          "state_key": "my-pack",
          "content": {
},
          "type": "im.ponies.room_emotes",
          "room_id": "!lfoDRlNFWlvOnvkBwQ:matrix.org"
        })"_json;

    EXPECT_NO_THROW(data2.get<ns::StateEvent<ns::msc2545::ImagePack>>());
}

TEST(StateEvents, Widget)
{
    json data = R"({
    "content": {
        "creatorUserId": "@rxl881:matrix.org",
        "data": {
            "title": "Bridges Dashboard",
            "dateRange": "1y"
        },
        "id": "grafana_@rxl881:matrix.org_1514573757015",
        "name": "Grafana",
        "type": "m.grafana",
        "url": "https://matrix.org/grafana/whatever",
        "waitForIframeLoad": true
    },
    "room_id": "!foo:bar",
    "event_id": "$15104760642668662QICBu:matrix.org",
    "sender": "@rxl881:matrix.org",
    "state_key": "grafana_@rxl881:matrix.org_1514573757015",
    "origin_server_ts": 1432735824653,
    "type": "m.widget"
})"_json;

    ns::StateEvent<ns::state::Widget> event = data.get<ns::StateEvent<ns::state::Widget>>();

    EXPECT_EQ(event.type, ns::EventType::Widget);
    EXPECT_EQ(event.event_id, "$15104760642668662QICBu:matrix.org");
    EXPECT_EQ(event.room_id, "!foo:bar");
    EXPECT_EQ(event.sender, "@rxl881:matrix.org");
    EXPECT_EQ(event.state_key, "grafana_@rxl881:matrix.org_1514573757015");
    EXPECT_EQ(event.content.creatorUserId, "@rxl881:matrix.org");
    EXPECT_EQ(event.content.name, "Grafana");
    EXPECT_EQ(event.content.type, "m.grafana");
    EXPECT_EQ(event.content.url, "https://matrix.org/grafana/whatever");
    EXPECT_EQ(event.content.waitForIframeLoad, true);
    ASSERT_EQ(event.content.data.size(), 2);
}

TEST(RoomEvents, OlmEncrypted)
{
    json data = R"({
          "content": {
            "algorithm": "m.olm.v1.curve25519-aes-sha2",
            "ciphertext": {
              "1OaiUJ7OfIEGAtnMQyTPFi9Ou6LD5UjSZ4eMk6WzI3E": {
                "body": "AwogkcAq9+r4YNvCwvBXmipeM30ZVhVDYBWPZ.......69/rEhCK38SIILvCA5NvEH",
                "type": 0
              }
            },
            "sender_key": "IlRMeOPX2e0MurIyfWEucYBRVOEEUMrOHqn/8mLqMjA"
          },
          "event_id": "$143273582443PhrSn:localhost",
          "origin_server_ts": 1432735824653,
          "room_id": "!jEsUZKDJdhlrceRyVU:localhost",
          "sender": "@example:localhost",
          "type": "m.room.encrypted",
          "unsigned": {
            "age": 146,
            "transaction_id": "m1476648745605.19"
          }
        })"_json;

    ns::EncryptedEvent<ns::msg::OlmEncrypted> event =
      data.get<ns::EncryptedEvent<ns::msg::OlmEncrypted>>();
    const auto key = event.content.ciphertext.begin()->first;

    EXPECT_EQ(event.type, ns::EventType::RoomEncrypted);
    EXPECT_EQ(event.event_id, "$143273582443PhrSn:localhost");
    EXPECT_EQ(event.room_id, "!jEsUZKDJdhlrceRyVU:localhost");
    EXPECT_EQ(event.sender, "@example:localhost");
    EXPECT_EQ(event.origin_server_ts, 1432735824653L);
    EXPECT_EQ(event.unsigned_data.age, 146);
    EXPECT_EQ(event.content.algorithm, "m.olm.v1.curve25519-aes-sha2");
    EXPECT_EQ(event.content.ciphertext.at(key).type, 0);
    EXPECT_EQ(event.content.ciphertext.at(key).body,
              "AwogkcAq9+r4YNvCwvBXmipeM30ZVhVDYBWPZ.......69/rEhCK38SIILvCA5NvEH");
    EXPECT_EQ(event.content.sender_key, "IlRMeOPX2e0MurIyfWEucYBRVOEEUMrOHqn/8mLqMjA");

    ns::msg::OlmEncrypted e1;
    e1.algorithm  = "m.olm.v1.curve25519-aes-sha2";
    e1.ciphertext = {{"1OaiUJ7OfIEGAtnMQyTPFi9Ou6LD5UjSZ4eMk6WzI3E",
                      {"AwogkcAq9+r4YNvCwvBXmipeM30ZVhVDYBWPZ.......69/rEhCK38SIILvCA5NvEH", 0}}};
    e1.sender_key = "IlRMeOPX2e0MurIyfWEucYBRVOEEUMrOHqn/8mLqMjA";

    json j = e1;
    ASSERT_EQ(
      j.dump(),
      "{\"algorithm\":\"m.olm.v1.curve25519-aes-sha2\","
      "\"ciphertext\":{\"1OaiUJ7OfIEGAtnMQyTPFi9Ou6LD5UjSZ4eMk6WzI3E\":{\"body\":\"AwogkcAq9+"
      "r4YNvCwvBXmipeM30ZVhVDYBWPZ.......69/rEhCK38SIILvCA5NvEH\",\"type\":0}},"
      "\"sender_key\":\"IlRMeOPX2e0MurIyfWEucYBRVOEEUMrOHqn/8mLqMjA\"}");
}

TEST(RoomEvents, Encrypted)
{
    json data = R"({
          "content": {
            "algorithm": "m.megolm.v1.aes-sha2",
            "ciphertext": "AwgAEnACgAkLmt6qF84IK++J7UDH2Za1YVchHyprqTqsg2yyOwAtHaZTwyNg37afzg8f3r9IsN9r4RNFg7MaZencUJe4qvELiDiopUjy5wYVDAtqdBzer5bWRD9ldxp1FLgbQvBcjkkywYjCsmsq6+hArLd9oAQZnGKn/qLsK+5uNX3PaWzDRC9wZPQvWYYPCTov3jCwXKTPsLKIiTrcCXDqMvnn8m+T3zF/I2zqxg158tnUwWWIw51UO",
            "device_id": "RJYKSTBOIE",
            "sender_key": "IlRMeOPX2e0MurIyfWEucYBRVOEEUMrOHqn/8mLqMjA",
            "session_id": "X3lUlvLELLYxeTx4yOVu6UDpasGEVO0Jbu+QFnm0cKQ"
          },
          "event_id": "$143273582443PhrSn:localhost",
          "origin_server_ts": 1432735824653,
          "room_id": "!jEsUZKDJdhlrceRyVU:localhost",
          "sender": "@example:localhost",
          "type": "m.room.encrypted",
          "unsigned": {
            "age": 146,
            "transaction_id": "m1476648745605.19"
          }
        })"_json;

    ns::EncryptedEvent<ns::msg::Encrypted> event =
      data.get<ns::EncryptedEvent<ns::msg::Encrypted>>();

    EXPECT_EQ(event.type, ns::EventType::RoomEncrypted);
    EXPECT_EQ(event.event_id, "$143273582443PhrSn:localhost");
    EXPECT_EQ(event.room_id, "!jEsUZKDJdhlrceRyVU:localhost");
    EXPECT_EQ(event.sender, "@example:localhost");
    EXPECT_EQ(event.origin_server_ts, 1432735824653L);
    EXPECT_EQ(event.unsigned_data.age, 146);
    EXPECT_EQ(event.content.algorithm, "m.megolm.v1.aes-sha2");
    EXPECT_EQ(
      event.content.ciphertext,
      "AwgAEnACgAkLmt6qF84IK++"
      "J7UDH2Za1YVchHyprqTqsg2yyOwAtHaZTwyNg37afzg8f3r9IsN9r4RNFg7MaZencUJe4qvELiDiopUjy5wYVDAt"
      "qdBzer5bWRD9ldxp1FLgbQvBcjkkywYjCsmsq6+hArLd9oAQZnGKn/"
      "qLsK+5uNX3PaWzDRC9wZPQvWYYPCTov3jCwXKTPsLKIiTrcCXDqMvnn8m+T3zF/I2zqxg158tnUwWWIw51UO");
    EXPECT_EQ(event.content.device_id, "RJYKSTBOIE");
    EXPECT_EQ(event.content.sender_key, "IlRMeOPX2e0MurIyfWEucYBRVOEEUMrOHqn/8mLqMjA");
    EXPECT_EQ(event.content.session_id, "X3lUlvLELLYxeTx4yOVu6UDpasGEVO0Jbu+QFnm0cKQ");

    ns::msg::Encrypted e1;
    e1.algorithm  = "m.megolm.v1.aes-sha2";
    e1.ciphertext = "AwgAEoABgw1DG6mgKwvrAJU+V7jPu3poEaujNWPnMtIO6+1kFHzEcK6vbYpbg/WlPq/"
                    "B23wqKWJ3DIaBsV305VdpisGK7dMN5WgnnTp9JhtztxpCuXnX92rWFBUFM9+"
                    "PC5xVJExVBm1qwv8xgWjD5NFqfcVsZ3jLGbGiftPHairq8bxPxTsjrblMHLpXyXLhK6A7YGTey"
                    "okcrdXS+IQ4Apq1RLP+kw5RF6M8a/aK3UhUlSAf7OLjaj/03qEwE3TGNaBbLBdOxzoGpxNfQ8";
    e1.device_id  = "YEGDJGLQTZ";
    e1.sender_key = "FyYq6RrnjvsIw0XLGF1jHYlorPgDmJQd15lMJw3D7QI";
    e1.session_id = "/bHcdWPHsJLFd8dkyvG0n7q/RTDmfBIc+gC4laHJCQQ";

    json j = e1;
    ASSERT_EQ(j.dump(),
              "{\"algorithm\":\"m.megolm.v1.aes-sha2\","
              "\"ciphertext\":\"AwgAEoABgw1DG6mgKwvrAJU+V7jPu3poEaujNWPnMtIO6+1kFHzEcK6vbYpbg/"
              "WlPq/B23wqKWJ3DIaBsV305VdpisGK7dMN5WgnnTp9JhtztxpCuXnX92rWFBUFM9"
              "+PC5xVJExVBm1qwv8xgWjD5NFqfcVsZ3jLGbGiftPHairq8bxPxTsjrblMHLpXyXLhK6A7YGTeyokcrdXS"
              "+IQ4Apq1RLP+kw5RF6M8a/aK3UhUlSAf7OLjaj/03qEwE3TGNaBbLBdOxzoGpxNfQ8\","
              "\"device_id\":\"YEGDJGLQTZ\","
              "\"sender_key\":\"FyYq6RrnjvsIw0XLGF1jHYlorPgDmJQd15lMJw3D7QI\","
              "\"session_id\":\"/bHcdWPHsJLFd8dkyvG0n7q/RTDmfBIc+gC4laHJCQQ\"}");
}

TEST(Ephemeral, Typing)
{
    json j = R"( {
    "content": {
        "user_ids": [
            "@alice:matrix.org",
            "@bob:example.com"
        ]
    },
    "room_id": "!jEsUZKDJdhlrceRyVU:example.org",
    "type": "m.typing"
})"_json;

    ns::EphemeralEvent<ns::ephemeral::Typing> event =
      j.get<ns::EphemeralEvent<ns::ephemeral::Typing>>();

    EXPECT_EQ(event.room_id, "!jEsUZKDJdhlrceRyVU:example.org");
    EXPECT_EQ(event.type, ns::EventType::Typing);
    EXPECT_EQ(event.content.user_ids.at(0), "@alice:matrix.org");
    EXPECT_EQ(event.content.user_ids.at(1), "@bob:example.com");
    EXPECT_EQ(j.dump(), json(event).dump());
}

TEST(Ephemeral, Receipt)
{
    json j = R"({
    "content": {
        "$1435641916114394fHBLK:matrix.org": {
            "m.read": {
                "@rikj:jki.re": {
                    "ts": 1436451550453
                }
            }
        }
    },
    "room_id": "!jEsUZKDJdhlrceRyVU:example.org",
    "type": "m.receipt"
})"_json;

    ns::EphemeralEvent<ns::ephemeral::Receipt> event =
      j.get<ns::EphemeralEvent<ns::ephemeral::Receipt>>();

    EXPECT_EQ(event.room_id, "!jEsUZKDJdhlrceRyVU:example.org");
    EXPECT_EQ(event.type, ns::EventType::Receipt);
    EXPECT_EQ(event.content.receipts.at("$1435641916114394fHBLK:matrix.org")
                .at(ns::ephemeral::Receipt::Read)
                .users.at("@rikj:jki.re")
                .ts,
              1436451550453);
    EXPECT_EQ(j.dump(), json(event).dump());
}

TEST(AccountData, Direct)
{
    json j = R"({
      "content": {
        "@bob:example.com": [
           "!abcdefgh:example.com",
           "!hgfedcba:example.com"
         ]
       },
       "type": "m.direct"
     })"_json;

    ns::AccountDataEvent<ns::account_data::Direct> event =
      j.get<ns::AccountDataEvent<ns::account_data::Direct>>();

    ASSERT_EQ(event.content.user_to_rooms.size(), 1);
    ASSERT_EQ(event.content.user_to_rooms.count("@bob:example.com"), 1);
    ASSERT_EQ(event.content.user_to_rooms["@bob:example.com"].size(), 2);
    ASSERT_EQ(event.content.user_to_rooms["@bob:example.com"][0], "!abcdefgh:example.com");
    ASSERT_EQ(event.content.user_to_rooms["@bob:example.com"][1], "!hgfedcba:example.com");
    EXPECT_EQ(j.dump(), json(event).dump());
}

TEST(AccountData, FullyRead)
{
    json j = R"({
            "content": {
            "event_id": "$someplace:example.org"
          },
          "room_id": "!somewhere:example.org",
          "type": "m.fully_read"
        })"_json;

    ns::AccountDataEvent<ns::account_data::FullyRead> event =
      j.get<ns::AccountDataEvent<ns::account_data::FullyRead>>();

    EXPECT_EQ(event.room_id, "!somewhere:example.org");
    EXPECT_EQ(event.type, ns::EventType::FullyRead);
    EXPECT_EQ(event.content.event_id, "$someplace:example.org");
    EXPECT_EQ(j.dump(), json(event).dump());
}

TEST(ToDevice, KeyVerificationRequest)
{
    json request_data = R"({
    "content": {
        "from_device": "AliceDevice2",
        "methods": [
            "m.sas.v1"
        ],
        "timestamp": 1559598944869,
        "transaction_id": "S0meUniqueAndOpaqueString"
    },
    "sender": "",
    "type": "m.key.verification.request"
})"_json;

    ns::DeviceEvent<ns::msg::KeyVerificationRequest> event =
      request_data.get<ns::DeviceEvent<ns::msg::KeyVerificationRequest>>();
    auto keyEvent = event.content;
    EXPECT_EQ(keyEvent.from_device, "AliceDevice2");
    EXPECT_EQ(event.type, mtx::events::EventType::KeyVerificationRequest);
    EXPECT_EQ(keyEvent.transaction_id, "S0meUniqueAndOpaqueString");
    EXPECT_EQ(keyEvent.methods[0], ns::msg::VerificationMethods::SASv1);
    EXPECT_EQ(keyEvent.timestamp, 1559598944869);
    EXPECT_EQ(request_data.dump(), json(event).dump());
}

TEST(ToDevice, KeyVerificationStart)
{
    json request_data = R"({
    "content": {
        "from_device": "BobDevice1",
        "hashes": [
            "sha256"
        ],
        "key_agreement_protocols": [
            "curve25519"
        ],
        "message_authentication_codes": [
            "hkdf-hmac-sha256"
        ],
        "method": "m.sas.v1",
        "short_authentication_string": [
            "decimal",
            "emoji",
            "some-random-invalid-method"
        ],
        "transaction_id": "S0meUniqueAndOpaqueString"
    },
    "sender": "",
    "type": "m.key.verification.start"
})"_json;

    ns::DeviceEvent<ns::msg::KeyVerificationStart> event =
      request_data.get<ns::DeviceEvent<ns::msg::KeyVerificationStart>>();
    auto keyEvent = event.content;
    EXPECT_EQ(keyEvent.from_device, "BobDevice1");
    EXPECT_EQ(keyEvent.hashes[0], "sha256");
    EXPECT_EQ(keyEvent.key_agreement_protocols[0], "curve25519");
    EXPECT_EQ(keyEvent.message_authentication_codes[0], "hkdf-hmac-sha256");
    EXPECT_EQ(keyEvent.short_authentication_string[0], ns::msg::SASMethods::Decimal);
    EXPECT_EQ(keyEvent.short_authentication_string[1], ns::msg::SASMethods::Emoji);
    EXPECT_EQ(keyEvent.short_authentication_string[2], ns::msg::SASMethods::Unsupported);
    EXPECT_EQ(event.type, mtx::events::EventType::KeyVerificationStart);
    EXPECT_EQ(keyEvent.transaction_id, "S0meUniqueAndOpaqueString");
    EXPECT_EQ(keyEvent.method, ns::msg::VerificationMethods::SASv1);
    // The incoming and outgoing JSON will not match due to the Unsupported SASMethod in the
    // request_data, so no point in comparing the dump of both for equality.
}

TEST(ToDevice, KeyVerificationAccept)
{
    json request_data = R"({
    "content": {
        "commitment": "fQpGIW1Snz+pwLZu6sTy2aHy/DYWWTspTJRPyNp0PKkymfIsNffysMl6ObMMFdIJhk6g6pwlIqZ54rxo8SLmAg",
        "hash": "sha256",
        "key_agreement_protocol": "curve25519",
        "message_authentication_code": "hkdf-hmac-sha256",
        "method": "m.sas.v1",
        "short_authentication_string": [
            "decimal",
            "emoji"
        ],
        "transaction_id": "S0meUniqueAndOpaqueString"
    },
    "sender": "",
    "type": "m.key.verification.accept"
})"_json;

    ns::DeviceEvent<ns::msg::KeyVerificationAccept> event =
      request_data.get<ns::DeviceEvent<ns::msg::KeyVerificationAccept>>();
    auto keyEvent = event.content;
    EXPECT_EQ(event.type, mtx::events::EventType::KeyVerificationAccept);
    EXPECT_EQ(
      keyEvent.commitment,
      "fQpGIW1Snz+pwLZu6sTy2aHy/DYWWTspTJRPyNp0PKkymfIsNffysMl6ObMMFdIJhk6g6pwlIqZ54rxo8SLmAg");
    EXPECT_EQ(keyEvent.hash, "sha256");
    EXPECT_EQ(keyEvent.key_agreement_protocol, "curve25519");
    EXPECT_EQ(keyEvent.message_authentication_code, "hkdf-hmac-sha256");
    EXPECT_EQ(keyEvent.short_authentication_string[0], ns::msg::SASMethods::Decimal);
    EXPECT_EQ(keyEvent.short_authentication_string[1], ns::msg::SASMethods::Emoji);
    EXPECT_EQ(keyEvent.transaction_id, "S0meUniqueAndOpaqueString");
    EXPECT_EQ(request_data.dump(), json(event).dump());
}

TEST(ToDevice, KeyVerificationReady)
{
    json request_data = R"({
    "content": {
      "from_device":"@alice:localhost",
      "methods":["m.sas.v1"],
      "transaction_id":"S0meUniqueAndOpaqueString"
    },
    "sender": "test_user",
    "type": "m.key.verification.ready"
})"_json;

    ns::DeviceEvent<ns::msg::KeyVerificationReady> event =
      request_data.get<ns::DeviceEvent<ns::msg::KeyVerificationReady>>();
    auto keyEvent = event.content;
    EXPECT_EQ(event.sender, "test_user");
    EXPECT_EQ(event.type, mtx::events::EventType::KeyVerificationReady);
    EXPECT_EQ(keyEvent.from_device, "@alice:localhost");
    EXPECT_EQ(keyEvent.transaction_id, "S0meUniqueAndOpaqueString");
    EXPECT_EQ(keyEvent.methods[0], ns::msg::VerificationMethods::SASv1);
}

TEST(ToDevice, KeyVerificationDone)
{
    json request_data = R"({
    "content": {
      "transaction_id": "S0meUniqueAndOpaqueString"
    },
    "sender": "test_user",
    "type": "m.key.verification.done"
})"_json;

    ns::DeviceEvent<ns::msg::KeyVerificationDone> event =
      request_data.get<ns::DeviceEvent<ns::msg::KeyVerificationDone>>();
    auto keyEvent = event.content;
    EXPECT_EQ(event.sender, "test_user");
    EXPECT_EQ(event.type, mtx::events::EventType::KeyVerificationDone);
    EXPECT_EQ(keyEvent.transaction_id, "S0meUniqueAndOpaqueString");
}

TEST(ToDevice, KeyVerificationCancel)
{
    json request_data = R"({
    "content": {
        "code": "m.user",
        "reason": "User rejected the key verification request",
        "transaction_id": "S0meUniqueAndOpaqueString"
    },
    "sender": "",
    "type": "m.key.verification.cancel"
})"_json;

    ns::DeviceEvent<ns::msg::KeyVerificationCancel> event =
      request_data.get<ns::DeviceEvent<ns::msg::KeyVerificationCancel>>();
    auto keyEvent = event.content;
    EXPECT_EQ(event.type, mtx::events::EventType::KeyVerificationCancel);
    EXPECT_EQ(keyEvent.code, "m.user");
    EXPECT_EQ(keyEvent.reason, "User rejected the key verification request");
    EXPECT_EQ(keyEvent.transaction_id, "S0meUniqueAndOpaqueString");
    EXPECT_EQ(request_data.dump(), json(event).dump());
}

TEST(ToDevice, KeyVerificationKey)
{
    json request_data = R"({
    "content": {
        "key": "fQpGIW1Snz+pwLZu6sTy2aHy/DYWWTspTJRPyNp0PKkymfIsNffysMl6ObMMFdIJhk6g6pwlIqZ54rxo8SLmAg",
        "transaction_id": "S0meUniqueAndOpaqueString"
    },
    "sender": "",
    "type": "m.key.verification.key"
})"_json;

    ns::DeviceEvent<ns::msg::KeyVerificationKey> event =
      request_data.get<ns::DeviceEvent<ns::msg::KeyVerificationKey>>();
    auto keyEvent = event.content;
    EXPECT_EQ(event.type, mtx::events::EventType::KeyVerificationKey);
    EXPECT_EQ(
      keyEvent.key,
      "fQpGIW1Snz+pwLZu6sTy2aHy/DYWWTspTJRPyNp0PKkymfIsNffysMl6ObMMFdIJhk6g6pwlIqZ54rxo8SLmAg");
    EXPECT_EQ(keyEvent.transaction_id, "S0meUniqueAndOpaqueString");
    EXPECT_EQ(request_data.dump(), json(event).dump());
}

TEST(ToDevice, KeyVerificationMac)
{
    json request_data = R"({
    "content": {
        "keys": "2Wptgo4CwmLo/Y8B8qinxApKaCkBG2fjTWB7AbP5Uy+aIbygsSdLOFzvdDjww8zUVKCmI02eP9xtyJxc/cLiBA",
        "mac": {
            "ed25519:ABCDEF": "fQpGIW1Snz+pwLZu6sTy2aHy/DYWWTspTJRPyNp0PKkymfIsNffysMl6ObMMFdIJhk6g6pwlIqZ54rxo8SLmAg"
        },
        "transaction_id": "S0meUniqueAndOpaqueString"
    },
    "sender": "",
    "type": "m.key.verification.mac"
})"_json;

    ns::DeviceEvent<ns::msg::KeyVerificationMac> event =
      request_data.get<ns::DeviceEvent<ns::msg::KeyVerificationMac>>();
    auto keyEvent = event.content;
    EXPECT_EQ(event.type, mtx::events::EventType::KeyVerificationMac);
    EXPECT_EQ(
      keyEvent.keys,
      "2Wptgo4CwmLo/Y8B8qinxApKaCkBG2fjTWB7AbP5Uy+aIbygsSdLOFzvdDjww8zUVKCmI02eP9xtyJxc/cLiBA");
    EXPECT_EQ(keyEvent.mac.count("ed25519:ABCDEF"), 1);
    EXPECT_EQ(
      keyEvent.mac.at("ed25519:ABCDEF"),
      "fQpGIW1Snz+pwLZu6sTy2aHy/DYWWTspTJRPyNp0PKkymfIsNffysMl6ObMMFdIJhk6g6pwlIqZ54rxo8SLmAg");
    EXPECT_EQ(keyEvent.transaction_id, "S0meUniqueAndOpaqueString");
    EXPECT_EQ(request_data.dump(), json(event).dump());
}

TEST(ToDevice, KeyRequest)
{
    json request_data = R"({
	"content": {
	  "action": "request",
	  "body": {
	    "algorithm": "m.megolm.v1.aes-sha2",
	    "room_id": "!iapLxlpZgOzqGnWkXR:matrix.org",
	    "sender_key": "9im1n0bSYQpnF700sXJqAAYiqGgkyRqMZRdobj0kymY",
	    "session_id": "oGj6sEDraRDf+NdmvZTI7urDJk/Z+i7TX2KFLbfMGlE"
	  },
	  "request_id": "m1529936829480.0",
	  "requesting_device_id": "GGUBYESVPI"
	},
        "sender": "@mujx:matrix.org",
        "type": "m.room_key_request"
	})"_json;
    mtx::events::DeviceEvent<ns::msg::KeyRequest> event(request_data);
    EXPECT_EQ(event.sender, "@mujx:matrix.org");
    EXPECT_EQ(event.type, mtx::events::EventType::RoomKeyRequest);
    EXPECT_EQ(event.content.action, ns::msg::RequestAction::Request);
    EXPECT_EQ(event.content.algorithm, "m.megolm.v1.aes-sha2");
    EXPECT_EQ(event.content.room_id, "!iapLxlpZgOzqGnWkXR:matrix.org");
    EXPECT_EQ(event.content.sender_key, "9im1n0bSYQpnF700sXJqAAYiqGgkyRqMZRdobj0kymY");
    EXPECT_EQ(event.content.session_id, "oGj6sEDraRDf+NdmvZTI7urDJk/Z+i7TX2KFLbfMGlE");
    EXPECT_EQ(event.content.request_id, "m1529936829480.0");
    EXPECT_EQ(event.content.requesting_device_id, "GGUBYESVPI");
    EXPECT_EQ(request_data.dump(), json(event).dump());
}

TEST(ToDevice, KeyCancellation)
{
    json cancellation_data = R"({
	  "content": {
            "action": "request_cancellation",
            "request_id": "m1529936829480.0",
            "requesting_device_id": "GGUBYESVPI"
          },
          "sender": "@mujx:matrix.org",
          "type": "m.room_key_request"
	})"_json;

    mtx::events::DeviceEvent<ns::msg::KeyRequest> event(cancellation_data);
    EXPECT_EQ(event.sender, "@mujx:matrix.org");
    EXPECT_EQ(event.type, mtx::events::EventType::RoomKeyRequest);
    EXPECT_EQ(event.content.action, ns::msg::RequestAction::Cancellation);
    EXPECT_EQ(event.content.request_id, "m1529936829480.0");
    EXPECT_EQ(event.content.requesting_device_id, "GGUBYESVPI");

    EXPECT_EQ(cancellation_data.dump(), json(event).dump());
}

TEST(Collection, Events)
{
    json data = R"({
	  "unsigned": {
	    "age": 242352,
	    "transaction_id": "txnid"
	  },
	  "content": {
	    "aliases": [
	      "#somewhere:localhost",
	      "#another:localhost"
	    ]
	  },
	  "event_id": "$WLGTSEFSEF:localhost",
	  "origin_server_ts": 1431961217939,
          "room_id": "!Cuyf34gef24t:localhost",
	  "sender": "@example:localhost",
	  "state_key": "localhost",
	  "type": "m.room.aliases"
	})"_json;

    mtx::events::collections::TimelineEvent event =
      data.get<mtx::events::collections::TimelineEvent>();

    ASSERT_TRUE(std::get_if<ns::StateEvent<ns::state::Aliases>>(&event.data) != nullptr);
}

TEST(RoomAccountData, Tags)
{
    json data = R"({
          "content": {
              "tags": {
                "m.favourite": {
                  "order": 1
                },
                "u.Project1": {
                  "order": 0
                },
                "com.example.nheko.text": {
                  "associated_data": ["some", "json", "list"]
                }
              }
          },
          "type": "m.tag"
        })"_json;

    ns::AccountDataEvent<ns::account_data::Tags> event =
      data.get<ns::AccountDataEvent<ns::account_data::Tags>>();

    EXPECT_EQ(event.type, ns::EventType::Tag);
    EXPECT_EQ(event.content.tags.size(), 3);
    EXPECT_EQ(event.content.tags.at("m.favourite").order, 1);
    EXPECT_EQ(event.content.tags.at("u.Project1").order, 0);
    // NOTE(Nico): We are currently not parsing arbitrary attached json data (anymore).
    EXPECT_EQ(event.content.tags.at("com.example.nheko.text").order, std::nullopt);
}

TEST(RoomAccountData, NhekoHiddenEvents)
{
    json data = R"({
          "content": {
              "hidden_event_types": [
	          "m.reaction",
		  "m.room.member"
	      ]
          },
          "type": "im.nheko.hidden_events"
        })"_json;

    ns::AccountDataEvent<ns::account_data::nheko_extensions::HiddenEvents> event =
      data.get<ns::AccountDataEvent<ns::account_data::nheko_extensions::HiddenEvents>>();

    EXPECT_EQ(event.type, ns::EventType::NhekoHiddenEvents);
    ASSERT_TRUE(event.content.hidden_event_types.has_value());
    ASSERT_EQ(event.content.hidden_event_types->size(), 2);
    EXPECT_EQ(event.content.hidden_event_types.value()[0], ns::EventType::Reaction);
    EXPECT_EQ(event.content.hidden_event_types.value()[1], ns::EventType::RoomMember);
}

TEST(RoomAccountData, ImagePack)
{
    json data = R"({
          "content": {
  "images": {
    "emote": {
      "url": "mxc://example.org/blah"
    },
    "sticker": {
      "url": "mxc://example.org/sticker",
      "body": "stcikerly",
      "usage": ["sticker"]
    }
  },
  "pack": {
    "display_name": "Awesome Pack",
    "avatar_url": "mxc://example.org/asdjfasd",
    "usage": ["emoticon"],
    "attribution": "huh"
  }
},
          "type": "im.ponies.user_emotes"
        })"_json;

    ns::AccountDataEvent<ns::msc2545::ImagePack> event =
      data.get<ns::AccountDataEvent<ns::msc2545::ImagePack>>();

    EXPECT_EQ(event.type, ns::EventType::ImagePackInAccountData);
    EXPECT_EQ(event.content.pack.has_value(), true);
    EXPECT_EQ(event.content.pack->display_name, "Awesome Pack");
    EXPECT_EQ(event.content.pack->attribution, "huh");
    EXPECT_EQ(event.content.pack->avatar_url, "mxc://example.org/asdjfasd");
    EXPECT_EQ(event.content.pack->is_emoji(), true);
    EXPECT_EQ(event.content.pack->is_sticker(), false);
    ASSERT_EQ(event.content.images.size(), 2);
    EXPECT_EQ(event.content.images["emote"].url, "mxc://example.org/blah");
    EXPECT_EQ(event.content.images["emote"].overrides_usage(), false);
    EXPECT_EQ(event.content.images["sticker"].url, "mxc://example.org/sticker");
    EXPECT_EQ(event.content.images["sticker"].body, "stcikerly");
    EXPECT_EQ(event.content.images["sticker"].is_sticker(), true);
    EXPECT_EQ(event.content.images["sticker"].is_emoji(), false);
    EXPECT_EQ(event.content.images["sticker"].overrides_usage(), true);
    EXPECT_EQ(json(event)["content"]["images"].size(), 2);
}

TEST(RoomAccountData, ImagePackRooms)
{
    json data = R"({
          "content": {
  "rooms": {
    "!someroom:example.org": {
      "": {},
      "de.sorunome.mx-puppet-bridge.discord": {}
    },
    "!someotherroom:example.org": {
      "": {}
    }
  }
},
          "type": "im.ponies.emote_rooms"
        })"_json;

    ns::AccountDataEvent<ns::msc2545::ImagePackRooms> event =
      data.get<ns::AccountDataEvent<ns::msc2545::ImagePackRooms>>();

    EXPECT_EQ(event.type, ns::EventType::ImagePackRooms);
    EXPECT_EQ(event.content.rooms.size(), 2);
    EXPECT_EQ(event.content.rooms["!someroom:example.org"].size(), 2);
    EXPECT_EQ(event.content.rooms["!someroom:example.org"].count(""), 1);
    EXPECT_EQ(
      event.content.rooms["!someroom:example.org"].count("de.sorunome.mx-puppet-bridge.discord"),
      1);
    EXPECT_EQ(json(event)["content"]["rooms"].size(), 2);

    ns::msc2545::ImagePackRooms empty = {};
    EXPECT_EQ(json(empty).dump(), "{\"rooms\":{}}");

    ns::msc2545::ImagePackRooms empty2 = json::object().get<ns::msc2545::ImagePackRooms>();
    EXPECT_TRUE(empty2.rooms.empty());
}

TEST(Presence, Presence)
{
    json data = R"({
	    "content": {
		"avatar_url": "mxc://localhost:wefuiwegh8742w",
		"currently_active": true,
		"last_active_ago": 2478593,
		"presence": "online",
		"status_msg": "Making cupcakes"
	    },
	    "sender": "@example:localhost",
	    "type": "m.presence"
	})"_json;

    ns::Event<ns::presence::Presence> event = data.get<ns::Event<ns::presence::Presence>>();

    EXPECT_EQ(event.type, ns::EventType::Presence);
    EXPECT_EQ(event.sender, "@example:localhost");
    EXPECT_EQ(event.content.avatar_url, "mxc://localhost:wefuiwegh8742w");
    EXPECT_EQ(event.content.currently_active, true);
    EXPECT_EQ(event.content.last_active_ago, 2478593);
    EXPECT_EQ(event.content.presence, mtx::presence::online);
    EXPECT_EQ(event.content.status_msg, "Making cupcakes");
    EXPECT_EQ(data, json(event));
}
