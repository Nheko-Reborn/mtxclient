#include <gtest/gtest.h>

#include <nlohmann/json.hpp>
#include <mtx.hpp>

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

        ns::RedactionEvent<ns::msg::Redaction> event = data;

        EXPECT_EQ(event.type, ns::EventType::RoomRedaction);
        EXPECT_EQ(event.event_id, "$143273582443PhrSn:localhost");
        EXPECT_EQ(event.room_id, "!jEsUZKDJdhlrceRyVU:localhost");
        EXPECT_EQ(event.sender, "@example:localhost");
        EXPECT_EQ(event.origin_server_ts, 1432735824653L);
        EXPECT_EQ(event.unsigned_data.age, 146);
        EXPECT_EQ(event.redacts, "$1521361675759563UDexf:matrix.org");

        EXPECT_EQ(event.content.reason, "No reason");
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
        EXPECT_EQ("m.room.redaction", ns::to_string(ns::EventType::RoomRedaction));
        EXPECT_EQ("m.room.pinned_events", ns::to_string(ns::EventType::RoomPinnedEvents));
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

        ns::StateEvent<ns::state::Aliases> aliases = data;

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

        ns::StateEvent<ns::state::Avatar> event = data;

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
          "origin_server_ts": 1506762071625,
          "sender": "@mujx:matrix.org",
          "event_id": "$15067620711415511reUFC:matrix.org",
          "age": 3717700323,
          "unsigned": {
            "age": 3717700323
          },
          "state_key": "",
          "content": {
            "alias": "#alias:matrix.org"
          },
          "room_id": "!VaMCVKSVcyPtXbcMXh:matrix.org",
          "user_id": "@mujx:matrix.org",
          "type": "m.room.canonical_alias"
        })"_json;

        ns::StateEvent<ns::state::CanonicalAlias> event = data;

        EXPECT_EQ(event.type, ns::EventType::RoomCanonicalAlias);
        EXPECT_EQ(event.event_id, "$15067620711415511reUFC:matrix.org");
        EXPECT_EQ(event.room_id, "!VaMCVKSVcyPtXbcMXh:matrix.org");
        EXPECT_EQ(event.sender, "@mujx:matrix.org");
        EXPECT_EQ(event.unsigned_data.age, 3717700323);
        EXPECT_EQ(event.origin_server_ts, 1506762071625L);
        EXPECT_EQ(event.state_key, "");
        EXPECT_EQ(event.content.alias, "#alias:matrix.org");
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

        ns::StateEvent<ns::state::Create> event = data;

        EXPECT_EQ(event.type, ns::EventType::RoomCreate);
        EXPECT_EQ(event.event_id, "$15067619231414398jhvQC:matrix.org");
        EXPECT_EQ(event.sender, "@mujx:matrix.org");
        EXPECT_EQ(event.unsigned_data.age, 3715756343L);
        EXPECT_EQ(event.origin_server_ts, 1506761923948L);
        EXPECT_EQ(event.state_key, "");
        EXPECT_EQ(event.content.creator, "@mujx:matrix.org");
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

        ns::StateEvent<ns::state::GuestAccess> event = data;

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

        ns::StateEvent<ns::state::HistoryVisibility> event = data;

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

        ns::StateEvent<ns::state::HistoryVisibility> event2 = data2;

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

        ns::StateEvent<ns::state::JoinRules> event = data;

        EXPECT_EQ(event.type, ns::EventType::RoomJoinRules);
        EXPECT_EQ(event.event_id, "$15067619241414401ASocy:matrix.org");
        EXPECT_EQ(event.sender, "@mujx:matrix.org");
        EXPECT_EQ(event.unsigned_data.age, 3715756273);
        EXPECT_EQ(event.origin_server_ts, 1506761924018L);
        EXPECT_EQ(event.state_key, "");
        EXPECT_EQ(event.content.join_rule, ns::state::JoinRule::Invite);
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

        ns::StateEvent<ns::state::Member> event = data;

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

        ns::StateEvent<ns::state::Member> event2 = data2;

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

        ns::StateEvent<ns::state::Name> event = data;

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

        ns::StateEvent<ns::state::PinnedEvents> event = data;

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

        ns::StateEvent<ns::state::PowerLevels> event = data;

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

        ns::StateEvent<ns::state::Topic> event = data;

        EXPECT_EQ(event.type, ns::EventType::RoomTopic);
        EXPECT_EQ(event.event_id, "$15104760642668662QICBu:matrix.org");
        EXPECT_EQ(event.room_id, "!lfoDRlNFWlvOnvkBwQ:matrix.org");
        EXPECT_EQ(event.sender, "@nheko_test:matrix.org");
        EXPECT_EQ(event.origin_server_ts, 1510476064445);
        EXPECT_EQ(event.unsigned_data.age, 37);
        EXPECT_EQ(event.state_key, "");
        EXPECT_EQ(event.content.topic, "Test topic");
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

        ns::EncryptedEvent<ns::msg::OlmEncrypted> event = data;
        const auto key                                  = event.content.ciphertext.begin()->first;

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
        e1.ciphertext = {
          {"1OaiUJ7OfIEGAtnMQyTPFi9Ou6LD5UjSZ4eMk6WzI3E",
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

        ns::EncryptedEvent<ns::msg::Encrypted> event = data;

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
        ASSERT_EQ(
          j.dump(),
          "{\"algorithm\":\"m.megolm.v1.aes-sha2\","
          "\"ciphertext\":\"AwgAEoABgw1DG6mgKwvrAJU+V7jPu3poEaujNWPnMtIO6+1kFHzEcK6vbYpbg/"
          "WlPq/B23wqKWJ3DIaBsV305VdpisGK7dMN5WgnnTp9JhtztxpCuXnX92rWFBUFM9"
          "+PC5xVJExVBm1qwv8xgWjD5NFqfcVsZ3jLGbGiftPHairq8bxPxTsjrblMHLpXyXLhK6A7YGTeyokcrdXS"
          "+IQ4Apq1RLP+kw5RF6M8a/aK3UhUlSAf7OLjaj/03qEwE3TGNaBbLBdOxzoGpxNfQ8\","
          "\"device_id\":\"YEGDJGLQTZ\","
          "\"sender_key\":\"FyYq6RrnjvsIw0XLGF1jHYlorPgDmJQd15lMJw3D7QI\","
          "\"session_id\":\"/bHcdWPHsJLFd8dkyvG0n7q/RTDmfBIc+gC4laHJCQQ\"}");
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

        ns::msg::KeyRequest event = request_data;
        EXPECT_EQ(event.sender, "@mujx:matrix.org");
        EXPECT_EQ(event.type, mtx::events::EventType::RoomKeyRequest);
        EXPECT_EQ(event.action, ns::msg::RequestAction::Request);
        EXPECT_EQ(event.algorithm, "m.megolm.v1.aes-sha2");
        EXPECT_EQ(event.room_id, "!iapLxlpZgOzqGnWkXR:matrix.org");
        EXPECT_EQ(event.sender_key, "9im1n0bSYQpnF700sXJqAAYiqGgkyRqMZRdobj0kymY");
        EXPECT_EQ(event.session_id, "oGj6sEDraRDf+NdmvZTI7urDJk/Z+i7TX2KFLbfMGlE");
        EXPECT_EQ(event.request_id, "m1529936829480.0");
        EXPECT_EQ(event.requesting_device_id, "GGUBYESVPI");

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

        ns::msg::KeyRequest event = cancellation_data;
        EXPECT_EQ(event.sender, "@mujx:matrix.org");
        EXPECT_EQ(event.type, mtx::events::EventType::RoomKeyRequest);
        EXPECT_EQ(event.action, ns::msg::RequestAction::Cancellation);
        EXPECT_EQ(event.request_id, "m1529936829480.0");
        EXPECT_EQ(event.requesting_device_id, "GGUBYESVPI");

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

        mtx::events::collections::TimelineEvent event = data;

        ASSERT_TRUE(boost::get<ns::StateEvent<ns::state::Aliases>>(&event.data) != nullptr);
}

TEST(RoomAccountData, Tag)
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

        ns::Event<ns::account_data::Tag> event = data;

        EXPECT_EQ(event.type, ns::EventType::Tag);
        EXPECT_EQ(event.content.tags.size(), 3);
        EXPECT_EQ(event.content.tags.at("m.favourite").at("order"), 1);
        EXPECT_EQ(event.content.tags.at("u.Project1").at("order"), 0);
        EXPECT_EQ(event.content.tags.at("com.example.nheko.text").at("associated_data").size(), 3);
}
