#include <gtest/gtest.h>

#include <fstream>
#include <variant>

#include <nlohmann/json.hpp>

#include <mtx.hpp>

#include "test_helpers.hpp"

using json = nlohmann::json;

using namespace mtx::responses;
using namespace mtx::events;

TEST(Responses, State)
{
        json data = R"({
	  "events": [
	    { 
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
	    },
	    { 
	      "unsigned": {
	        "age": 242352,
	        "transaction_id": "txnid"
	      },
	      "content": {
 	        "name": "Random name"
	      },
	      "event_id": "$WLGTSEFSEF2:localhost",
	      "origin_server_ts": 1431961217939,
              "room_id": "!Cuyf34gef24t:localhost",
	      "sender": "@example2:localhost",
	      "state_key": "localhost",
	      "type": "m.room.name"
	    }
	  ]
	})"_json;

        State state = data;

        EXPECT_EQ(state.events.size(), 2);

        auto aliases = std::get<StateEvent<state::Aliases>>(state.events[0]);
        EXPECT_EQ(aliases.event_id, "$WLGTSEFSEF:localhost");
        EXPECT_EQ(aliases.type, EventType::RoomAliases);
        EXPECT_EQ(aliases.sender, "@example:localhost");
        EXPECT_EQ(aliases.content.aliases.size(), 2);
        EXPECT_EQ(aliases.content.aliases[0], "#somewhere:localhost");

        auto name = std::get<StateEvent<state::Name>>(state.events[1]);
        EXPECT_EQ(name.event_id, "$WLGTSEFSEF2:localhost");
        EXPECT_EQ(name.type, EventType::RoomName);
        EXPECT_EQ(name.sender, "@example2:localhost");
        EXPECT_EQ(name.content.name, "Random name");

        // The first event is malformed (has null as the user id)
        // and therefore is should be skipped.
        json malformed_data = R"({
	  "events": [
	    { 
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
	      "sender": null,
	      "state_key": "localhost",
	      "type": "m.room.aliases"
	    },
	    { 
	      "unsigned": {
	        "age": 242352,
	        "transaction_id": "txnid"
	      },
	      "content": {
 	        "name": "Random name"
	      },
	      "event_id": "$WLGTSEFSEF2:localhost",
	      "origin_server_ts": 1431961217939,
              "room_id": "!Cuyf34gef24t:localhost",
	      "sender": "@example2:localhost",
	      "state_key": "localhost",
	      "type": "m.room.name"
	    }
	  ]
	})"_json;

        State malformed_state = malformed_data;

        EXPECT_EQ(malformed_state.events.size(), 1);

        name = std::get<StateEvent<state::Name>>(malformed_state.events[0]);
        EXPECT_EQ(name.event_id, "$WLGTSEFSEF2:localhost");
        EXPECT_EQ(name.type, EventType::RoomName);
        EXPECT_EQ(name.sender, "@example2:localhost");
        EXPECT_EQ(name.content.name, "Random name");
}

TEST(Responses, Timeline) {}
TEST(Responses, JoinedRoom)
{
        json data1 = R"({
            "ephemeral": {
                "events": [
                    {
                        "content": {
                            "$123456789123456789ABC:matrix.org": {
                                "m.read": {
                                    "@user1:s1.example.com": {
                                        "ts": 1515754170039
                                    },
                                    "@user2:s2.example.com": {
                                        "ts": 1515713767417
                                    }
                                }
                            }
                        },
                        "type": "m.receipt"
                    }
                ]
            },
            "unread_notifications": {
                "highlight_count": 2,
                "notification_count": 4
            }
	})"_json;

        JoinedRoom room1 = data1;

        // It this succeeds parsing was done successfully
        EXPECT_EQ(room1.ephemeral.events.size(), 1);
        EXPECT_EQ(room1.timeline.events.size(), 0);
        EXPECT_EQ(room1.unread_notifications.highlight_count, 2);
        EXPECT_EQ(room1.unread_notifications.notification_count, 4);

        json data2 = R"({
            "timeline": {
                "events": [
                    {
                        "content": {
                            "avatar_url": "mxc://matrix.org/MatrixContentID123456789",
                            "displayname": "DisplayName",
                            "membership": "join"
                        },
                        "event_id": "$1234567898765432123456:matrix.org",
                        "membership": "join",
                        "origin_server_ts": 1515756721018,
                        "sender": "@user1:s1.example.com",
                        "state_key": "@user1:s1.example.com",
                        "type": "m.room.member",
                        "unsigned": {
                            "age": 18585,
                            "prev_content": {
                                "avatar_url": null,
                                "displayname": "DisplayName",
                                "membership": "join"
                            },
                            "prev_sender": "@user1:s1.example.com",
                            "replaces_state": "$1234567898765432123455:matrix.org"
                        }
                    },
                    {
                        "content": {
                            "body": "Hello World",
                            "msgtype": "m.text"
                        },
                        "event_id": "$12345678987654321asdfg:matrix.org",
                        "origin_server_ts": 1515756732500,
                        "sender": "@user1:s1.example.com",
                        "type": "m.room.message",
                        "unsigned": {
                            "age": 8032
                        }
                    }
                ],
                "limited": false,
                "prev_batch": "s42_42_42_42_42_42_42_42_1"
            },
            "unread_notifications": {
                "highlight_count": 2,
                "notification_count": 4
            }
	})"_json;

        JoinedRoom room2 = data2;
        EXPECT_EQ(room2.ephemeral.events.size(), 0);
        EXPECT_EQ(room2.timeline.events.size(), 2);
        EXPECT_EQ(room2.timeline.prev_batch, "s42_42_42_42_42_42_42_42_1");
        EXPECT_EQ(room2.unread_notifications.highlight_count, 2);
        EXPECT_EQ(room2.unread_notifications.notification_count, 4);
}
TEST(Responses, LeftRoom)
{
        json data = R"({
            "timeline": {
                "events": [
                    {
                        "content": {
                            "membership": "leave"
                        },
                        "event_id": "$12345678923456789:s1.example.com",
                        "membership": "leave",
                        "origin_server_ts": 1234567894342,
                        "sender": "@u1:s1.example.com",
                        "state_key": "@u1:s1.example.com",
                        "type": "m.room.member",
                        "unsigned": {
                            "age": 1566,
                            "prev_content": {
                                "avatar_url": "mxc://msgs.tk/MatrixContentId123456789",
                                "displayname": "User 1!",
                                "membership": "join"
                            },
                            "prev_sender": "@u1:s1.example.com",
                            "replaces_state": "$12345678912345678:s1.example.com"
                        }
                    }
                ],
                "limited": false,
                "prev_batch": "s123_42_1234_4321123_13579_12_14400_4221_7"
            }
	})"_json;

        LeftRoom room = data;

        EXPECT_EQ(room.timeline.events.size(), 1);
        EXPECT_EQ(room.timeline.limited, false);
        EXPECT_EQ(room.state.events.size(), 0);
}

TEST(Responses, InvitedRoom)
{
        json data = R"({
	"invite_state": {
	  "events":[{
	    "content":{
	      "name":"Testing room"
	    },
	    "sender":"@mujx:matrix.org",
	    "state_key":"",
	    "type":"m.room.name"
	  },{
	    "content":{"url":"mxc://matrix.org/wdjzHdrThpqWyVArfyWmRbBx"},
	    "sender":"@mujx:matrix.org",
	    "state_key":"",
	    "type":"m.room.avatar"
	  },{
	    "content":{
	      "avatar_url":"mxc://matrix.org/JKiSOBDDxCHxmaLAgoQwSAHa",
	      "displayname":"NhekoTest",
	      "membership":"join"
	    },
	    "sender":"@nheko_test:matrix.org",
	    "state_key":"@nheko_test:matrix.org",
	    "type":"m.room.member"
	  },{
	    "content":{"alias":"#tessssssst:matrix.org"},
	    "sender":"@mujx:matrix.org",
	    "state_key":"",
	    "type":"m.room.canonical_alias"},
	  {
	    "content":{"join_rule":"invite"},
	    "sender":"@mujx:matrix.org",
	    "state_key":"",
	    "type":"m.room.join_rules"
	  },{
	    "content":{"avatar_url":"mxc://matrix.org/mGOKULXnAOnyplROyaxQcyoC",
	    "displayname":"mujx",
	    "membership":"invite"
	  },
	    "event_id":"$1513636848665012cjHwG:matrix.org",
	    "membership":"invite",
	    "origin_server_ts":1513636848325,
	    "sender":"@nheko_test:matrix.org",
	    "state_key":"@mujx:matrix.org",
	    "type":"m.room.member",
	    "unsigned":{
	      "age":279,
	      "prev_content":{"membership":"leave"},
	      "prev_sender":"@mujx:matrix.org",
	      "replaces_state":"$15068762701126850oGdvT:matrix.org"
	    }
	  }
	]}}
	)"_json;

        InvitedRoom room = data;

        EXPECT_EQ(room.invite_state.size(), 6);

        auto name = std::get<StrippedEvent<state::Name>>(room.invite_state[0]);
        EXPECT_EQ(name.type, EventType::RoomName);
        EXPECT_EQ(name.content.name, "Testing room");

        auto avatar = std::get<StrippedEvent<state::Avatar>>(room.invite_state[1]);
        EXPECT_EQ(avatar.type, EventType::RoomAvatar);
        EXPECT_EQ(avatar.content.url, "mxc://matrix.org/wdjzHdrThpqWyVArfyWmRbBx");
}

TEST(Responses, Sync)
{
        std::ifstream file(fixture_prefix() + "/fixtures/responses/sync.json");

        json data1;
        file >> data1;

        Sync sync1 = data1;

        EXPECT_EQ(sync1.next_batch,
                  "s333358558_324502987_444424_65663508_21685260_193623_2377336_2940807_454");
        EXPECT_EQ(sync1.rooms.join.size(), 5);

        auto nheko = sync1.rooms.join["!BPvgRcBVHzyFSlYkrg:matrix.org"];
        EXPECT_GT(nheko.state.events.size(), 0);
        EXPECT_GT(nheko.timeline.events.size(), 0);
        EXPECT_EQ(nheko.timeline.limited, true);
        EXPECT_EQ(nheko.timeline.prev_batch,
                  "t10853-333025362_324502987_444424_65663508_21685260_193623_2377336_2940807_454");
        EXPECT_EQ(nheko.account_data.events.size(), 2);

        EXPECT_EQ(sync1.rooms.leave.size(), 1);
        EXPECT_EQ(sync1.rooms.invite.size(), 0);

        // Check consistency of incomplete sync
        json data2 = R"({
            "device_one_time_keys_count": {},
            "next_batch": "s123_42_42_42_42_42_42_42_7"
	})"_json;

        Sync sync2 = data2;

        EXPECT_EQ(sync2.next_batch, "s123_42_42_42_42_42_42_42_7");
        EXPECT_EQ(sync2.rooms.join.size(), 0);
        EXPECT_EQ(sync2.rooms.leave.size(), 0);
        EXPECT_EQ(sync2.rooms.invite.size(), 0);
}

TEST(Responses, SyncWithEncryption)
{
        std::ifstream file(fixture_prefix() + "/fixtures/responses/sync_with_crypto.json");

        json data;
        file >> data;

        Sync sync = data;

        EXPECT_EQ(sync.device_lists.changed.size(), 1);
        EXPECT_EQ(sync.device_lists.changed.at(0), "@carl:matrix.org");

        EXPECT_EQ(sync.device_lists.left.size(), 2);
        EXPECT_EQ(sync.device_lists.left.at(0), "@alice:matrix.org");
        EXPECT_EQ(sync.device_lists.left.at(1), "@bob:matrix.org");

        EXPECT_EQ(sync.device_one_time_keys_count.size(), 2);
        EXPECT_EQ(sync.device_one_time_keys_count["curve25519"], 10);
        EXPECT_EQ(sync.device_one_time_keys_count["signed_curve25519"], 50);

        auto timeline_events = sync.rooms.join.begin()->second.timeline.events;

        std::string algorithm_found;
        std::string event_id;
        for (const auto &e : timeline_events) {
                if (auto enc_event = std::get_if<StateEvent<mtx::events::state::Encryption>>(&e);
                    enc_event != nullptr) {
                        algorithm_found = enc_event->content.algorithm;
                        event_id        = enc_event->event_id;
                }
        }

        EXPECT_EQ(algorithm_found, "m.megolm.v1.aes-sha2");
        EXPECT_EQ(event_id, "$1522842442112652dsEBQ:matrix.org");
}

TEST(Responses, Rooms) {}

TEST(Responses, Profile)
{
        json response = R"({
	  "avatar_url": "mxc://matrix.org/SDGdghriugerRg",
	  "displayname": "Alice Margatroid"
        })"_json;

        json null_response = R"({
	  "avatar_url": "mxc://matrix.org/SDGdghriugerRg",
	  "displayname": null
        })"_json;

        json missing_response = R"({
	  "displayname": "Alice Margatroid"
        })"_json;

        json error_response = R"({
	  "displayname": 42
        })"_json;

        Profile profile = response;
        EXPECT_EQ(profile.avatar_url, "mxc://matrix.org/SDGdghriugerRg");
        EXPECT_EQ(profile.display_name, "Alice Margatroid");

        Profile null_profile = null_response;
        EXPECT_EQ(null_profile.avatar_url, "mxc://matrix.org/SDGdghriugerRg");
        EXPECT_EQ(null_profile.display_name, "");

        Profile missing_profile = missing_response;
        EXPECT_EQ(missing_profile.avatar_url, "");
        EXPECT_EQ(missing_profile.display_name, "Alice Margatroid");

        ASSERT_THROW(Profile error_profile = error_response, std::exception);
}

TEST(Responses, Versions)
{
        json data = R"({
	  "versions" : [
	    "r0.0.1",
	    "r0.2.0",
	    "r0.3.0"
	  ]
        })"_json;

        Versions versions = data;
        EXPECT_EQ(versions.versions.size(), 3);
        EXPECT_EQ(versions.versions[0], "r0.0.1");
        EXPECT_EQ(versions.versions[1], "r0.2.0");
        EXPECT_EQ(versions.versions[2], "r0.3.0");

        json error_data = R"({
	  "versions" : [
	    "r.0.0.1"
	  ]
        })"_json;

        ASSERT_THROW(Versions versions = error_data, std::invalid_argument);
}

TEST(Responses, WellKnown)
{
        json data = R"({
          "m.homeserver": {
            "base_url": "https://matrix.example.com"
          },
          "m.identity_server": {
            "base_url": "https://identity.example.com"
          },
          "org.example.custom.property": {
            "app_url": "https://custom.app.example.org"
          }
        })"_json;

        WellKnown wellknown = data;
        EXPECT_EQ(wellknown.homeserver.base_url, "https://matrix.example.com");
        EXPECT_EQ(wellknown.identity_server->base_url, "https://identity.example.com");
}

TEST(Responses, CreateRoom)
{
        json data = R"({"room_id" : "!sefiuhWgwghwWgh:example.com"})"_json;

        mtx::responses::CreateRoom create_room = data;
        EXPECT_EQ(create_room.room_id.to_string(), "!sefiuhWgwghwWgh:example.com");

        json error_data = R"({"room_id" : "#akajdkf:example.com"})"_json;

        ASSERT_THROW(CreateRoom create_room = error_data, std::invalid_argument);
}

TEST(Responses, Login)
{
        json data = R"({
          "user_id": "@cheeky_monkey:matrix.org",
          "access_token": "abc123", 
	  "home_server": "matrix.org",
          "device_id": "GHTYAJCE",
	  "well_known": {
	     "m.homeserver": {
	       "base_url": "https://example.org"
	     },
	     "m.identity_server": {
	       "base_url": "https://id.example.org"
	     }
	  }
        })"_json;

        Login login = data;
        EXPECT_EQ(login.user_id.to_string(), "@cheeky_monkey:matrix.org");
        EXPECT_EQ(login.access_token, "abc123");
        EXPECT_EQ(login.device_id, "GHTYAJCE");
        EXPECT_EQ(login.well_known->homeserver.base_url, "https://example.org");
        EXPECT_EQ(login.well_known->identity_server->base_url, "https://id.example.org");

        json data2 = R"({
          "user_id": "@cheeky_monkey:matrix.org",
          "access_token": "abc123", 
	  "home_server": "matrix.org"
        })"_json;

        Login login2 = data2;
        EXPECT_EQ(login2.user_id.to_string(), "@cheeky_monkey:matrix.org");
        EXPECT_EQ(login2.access_token, "abc123");
        EXPECT_EQ(login2.device_id, "");

        json data3   = R"({
          "user_id": "@cheeky_monkey:matrix.org",
          "access_token": "abc123"
        })"_json;
        Login login3 = data3;
        EXPECT_EQ(login3.user_id.to_string(), "@cheeky_monkey:matrix.org");
        EXPECT_EQ(login3.access_token, "abc123");
        EXPECT_EQ(login3.device_id, "");
}

TEST(Responses, Messages)
{
        json data = R"({
	"start": "t47429-4392820_219380_26003_2265",
	"end": "t47409-4357353_219380_26003_2265",
	"chunk": [{
	  "origin_server_ts": 1444812213737,
	  "sender": "@alice:example.com",
	  "event_id": "$1444812213350496Caaaa:example.com",
	  "content": {
	    "body": "hello world",
	    "msgtype": "m.text"
	  },
	  "room_id": "!Xq3620DUiqCaoxq:example.com",
	  "type": "m.room.message",
	  "age": 1042
	}, {
	  "origin_server_ts": 1444812194656,
	  "sender": "@bob:example.com",
	  "event_id": "$1444812213350496Cbbbb:example.com",
	  "content": {
	    "body": "the world is big",
	    "msgtype": "m.text"
	  },
	  "room_id": "!Xq3620DUiqCaoxq:example.com",
	  "type": "m.room.message",
	  "age": 20123
	}, {
	  "origin_server_ts": 1444812163990,
	  "sender": "@bob:example.com",
	  "event_id": "$1444812213350496Ccccc:example.com",
	  "content": {
	    "name": "New room name"
	  },
	  "prev_content": {
	    "name": "Old room name"
	  },
	  "state_key": "",
	  "room_id": "!Xq3620DUiqCaoxq:example.com",
	  "type": "m.room.name",
	  "age": 50789
	 }
	]})"_json;

        Messages messages = data;
        EXPECT_EQ(messages.start, "t47429-4392820_219380_26003_2265");
        EXPECT_EQ(messages.end, "t47409-4357353_219380_26003_2265");
        EXPECT_EQ(messages.chunk.size(), 3);

        using mtx::events::RoomEvent;
        using mtx::events::StateEvent;
        using mtx::events::msg::Text;
        using mtx::events::state::Name;

        auto first_event = std::get<RoomEvent<Text>>(messages.chunk[0]);
        EXPECT_EQ(first_event.content.body, "hello world");
        EXPECT_EQ(first_event.content.msgtype, "m.text");
        EXPECT_EQ(first_event.type, mtx::events::EventType::RoomMessage);
        EXPECT_EQ(first_event.event_id, "$1444812213350496Caaaa:example.com");

        auto second_event = std::get<RoomEvent<Text>>(messages.chunk[1]);
        EXPECT_EQ(second_event.content.body, "the world is big");
        EXPECT_EQ(second_event.content.msgtype, "m.text");
        EXPECT_EQ(second_event.type, mtx::events::EventType::RoomMessage);
        EXPECT_EQ(second_event.event_id, "$1444812213350496Cbbbb:example.com");

        auto third_event = std::get<StateEvent<Name>>(messages.chunk[2]);
        EXPECT_EQ(third_event.content.name, "New room name");
        EXPECT_EQ(third_event.type, mtx::events::EventType::RoomName);
        EXPECT_EQ(third_event.event_id, "$1444812213350496Ccccc:example.com");
        EXPECT_EQ(third_event.sender, "@bob:example.com");

        // Two of the events are malformed and should be dropped.
        // 1. Missing "type" key.
        // 2. Content is null.
        json malformed_data = R"({
	"start": "t47429-4392820_219380_26003_2265",
	"end": "t47409-4357353_219380_26003_2265",
	"chunk": [{
	  "origin_server_ts": 1444812213737,
	  "sender": "@alice:example.com",
	  "event_id": "$1444812213350496Caaaa:example.com",
	  "content": {
	    "body": "hello world",
	    "msgtype": "m.text"
	  },
	  "room_id": "!Xq3620DUiqCaoxq:example.com",
	  "age": 1042
	}, {
	  "origin_server_ts": 1444812194656,
	  "sender": "@bob:example.com",
	  "event_id": "$1444812213350496Cbbbb:example.com",
	  "content": null,
	  "room_id": "!Xq3620DUiqCaoxq:example.com",
	  "type": "m.room.message",
	  "age": 20123
	}, {
	  "origin_server_ts": 1444812163990,
	  "sender": "@bob:example.com",
	  "event_id": "$1444812213350496Ccccc:example.com",
	  "content": {
	    "name": "New room name"
	  },
	  "prev_content": {
	    "name": "Old room name"
	  },
	  "state_key": "",
	  "room_id": "!Xq3620DUiqCaoxq:example.com",
	  "type": "m.room.name",
	  "age": 50789
	 }
	]})"_json;

        messages = malformed_data;
        EXPECT_EQ(messages.start, "t47429-4392820_219380_26003_2265");
        EXPECT_EQ(messages.end, "t47409-4357353_219380_26003_2265");
        EXPECT_EQ(messages.chunk.size(), 1);

        third_event = std::get<StateEvent<Name>>(messages.chunk[0]);
        EXPECT_EQ(third_event.content.name, "New room name");
        EXPECT_EQ(third_event.type, mtx::events::EventType::RoomName);
        EXPECT_EQ(third_event.event_id, "$1444812213350496Ccccc:example.com");
        EXPECT_EQ(third_event.sender, "@bob:example.com");
}

TEST(Responses, EphemeralTyping)
{
        json data = R"({
          "events": [{
            "type": "m.typing",
            "content": {
              "user_ids": [
                "@alice:example.com",
                "@bob:example.com"
              ]
            }
          }]
        })"_json;

        mtx::responses::Ephemeral ephemeral = data;

        EXPECT_EQ(ephemeral.events.size(), 1);
        ASSERT_TRUE(
          std::holds_alternative<mtx::events::EphemeralEvent<mtx::events::ephemeral::Typing>>(
            ephemeral.events[0]));
        auto e = std::get<mtx::events::EphemeralEvent<mtx::events::ephemeral::Typing>>(
          ephemeral.events[0]);

        EXPECT_EQ(e.content.user_ids.size(), 2);
        EXPECT_EQ(e.content.user_ids[0], "@alice:example.com");
        EXPECT_EQ(e.content.user_ids[1], "@bob:example.com");
}

TEST(Responses, EphemeralReceipts)
{
        json data = R"({
          "events": [{
            "type": "m.typing",
            "content": {
              "user_ids": [
                "@alice:example.com",
                "@bob:example.com"
              ]
            }
          }, {
	    "type": "m.receipt",
	    "content": {
	      "$1493012095444993JeMrW:matrix.org": {
		"m.read": {
		  "@trilobite17:matrix.org": { "ts": 1493020945945 }
		}
	      },
	      "$1493135885261887UVyOW:matrix.org": {
	        "m.read": {
		  "@aaron:matrix.org": { "ts": 1493161552008 }
		}
	      },
	      "$149339947230ohuCC:krtdex.com": {
	        "m.read": {
		  "@walle303:matrix.eastcoast.hosting": { "ts": 1493404654684 }
		}
	      },
	      "$1493556582917fOMpi:vurpo.fi": {
		"m.read": {
	  	  "@matthew2:matrix.org": { "ts": 1493557057338 }
		}
	      },
	      "$14935874261161012PaoJD:matrix.org": {
		"m.read": {
		  "@frantisek:gajdusek.net": { "ts": 1493623595682 },
		  "@PhoenixLandPirate:matrix.org": { "ts": 1510630539168 },
		  "@Tokodomo:matrix.org": { "ts": 1510588032780 },
		  "@matthew:matrix.org": { "ts": 1510440324233 },
		  "@memoryruins:matrix.org": { "ts": 1510518443679 },
		  "@nagua:2hg.org": { "ts": 1510451215569 },
		  "@nioshd:matrix.org": { "ts": 1510521086750 }
		}
	      }
	    }
	  }]
        })"_json;

        mtx::responses::Ephemeral ephemeral = data;

        EXPECT_EQ(ephemeral.events.size(), 2);
        ASSERT_TRUE(
          std::holds_alternative<mtx::events::EphemeralEvent<mtx::events::ephemeral::Typing>>(
            ephemeral.events[0]));
        auto e = std::get<mtx::events::EphemeralEvent<mtx::events::ephemeral::Typing>>(
          ephemeral.events[0]);

        EXPECT_EQ(e.content.user_ids.size(), 2);
        EXPECT_EQ(e.content.user_ids[0], "@alice:example.com");
        EXPECT_EQ(e.content.user_ids[1], "@bob:example.com");

        ASSERT_TRUE(
          std::holds_alternative<mtx::events::EphemeralEvent<mtx::events::ephemeral::Receipt>>(
            ephemeral.events[1]));
        auto read = std::get<mtx::events::EphemeralEvent<mtx::events::ephemeral::Receipt>>(
          ephemeral.events[1]);

        EXPECT_EQ(read.content.receipts.size(), 5);
        EXPECT_EQ(read.content.receipts["$149339947230ohuCC:krtdex.com"].users.size(), 1);
        EXPECT_EQ(read.content.receipts["$14935874261161012PaoJD:matrix.org"].users.size(), 7);
        EXPECT_EQ(read.content.receipts["$14935874261161012PaoJD:matrix.org"]
                    .users["@matthew:matrix.org"]
                    .ts,
                  1510440324233);
}

TEST(Responses, Empty)
{
        json data = R"({})"_json;

        Empty e = data;
        (void)e;
}

TEST(Responses, Media)
{
        json data = R"({
	  "content_uri": "mxc://example.com/AQwafuaFswefuhsfAFAgsw"
	})"_json;

        ContentURI res = data;
        EXPECT_EQ(res.content_uri, "mxc://example.com/AQwafuaFswefuhsfAFAgsw");
}

TEST(Responses, UploadKeys)
{
        json data = R"({
	  "one_time_key_counts": {
            "curve25519": 10,
            "signed_curve25519": 20
          }
	})"_json;

        UploadKeys res = data;

        EXPECT_EQ(res.one_time_key_counts.size(), 2);
        EXPECT_EQ(res.one_time_key_counts["curve25519"], 10);
        EXPECT_EQ(res.one_time_key_counts["signed_curve25519"], 20);
}

TEST(Responses, QueryKeys)
{
        json data = R"({
      "failures": {
	      "noidea": { "what": 0 },
	      "toput": { "here": 1 }
	    },
      "master_keys": {
          "@alice:example.org": {
            "user_id": "@alice:example.org",
            "usage": ["master_keys"],
            "keys": {
              "ed25519:base64+self+signing+public+key": "base64+self+signing+public+key"
            },
            "signatures": {
              "@alice:example.org": {
                "ed25519:base64+device+id": "signature+of+master+key"
              }
            }
          }
        },
        "user_signing_keys": {
            "@alice:example.org": {
              "user_id": "@alice:example.org",
              "usage": ["user_signing"],
              "keys": {
                "ed25519:base64+user+signing+public+key": "base64+user+signing+public+key"
              },
              "signatures": {
                "@alice:example.org": {
                  "ed25519:base64+master+public+key": "signature+of+user+signing+key"
                }
              }
            }
          },
        "self_signing_keys": {
            "@alice:example.org": {
              "user_id": "@alice:example.org",
              "usage": ["self_signing"],
              "keys": {
                "ed25519:base64+self+signing+public+key": "base64+self+signing+public+key"
              },
              "signatures": {
                "@alice:example.org": {
                  "ed25519:base64+master+public+key": "signature+of+self+signing+key"
                }
              }
            }
          },
          "device_keys": {
            "@alice:example.com": {
              "JLAFKJWSCS": {
                "user_id": "@alice:example.com",
                "device_id": "JLAFKJWSCS",
                "algorithms": [ "m.olm.v1.curve25519-aes-sha2", "m.megolm.v1.aes-sha2" ],
                "keys": {
                  "curve25519:JLAFKJWSCS": "3C5BFWi2Y8MaVvjM8M22DBmh24PmgR0nPvJOIArzgyI",
                  "ed25519:JLAFKJWSCS": "lEuiRJBit0IG6nUf5pUzWTUEsRVVe/HJkoKuEww9ULI"
                },
                "signatures": {
                  "@alice:example.com": {
                    "ed25519:JLAFKJWSCS": "dSO80A01XiigH3uBiDVx/EjzaoycHcjq9lfQX0uWsqxl2giMIiSPR8a4d291W1ihKJL/a+myXS367WT6NAIcBA"
                  }
                },
                "unsigned": {
                  "device_display_name": "Alice's mobile phone"
                }
              }
            }
          }
        })"_json;

        QueryKeys res = data;

        EXPECT_EQ(res.failures.size(), 2);
        EXPECT_EQ(res.device_keys.size(), 1);

        auto device_keys = res.device_keys["@alice:example.com"]["JLAFKJWSCS"];

        EXPECT_EQ(device_keys.user_id, "@alice:example.com");
        EXPECT_EQ(device_keys.device_id, "JLAFKJWSCS");
        EXPECT_EQ(device_keys.algorithms[0], "m.olm.v1.curve25519-aes-sha2");
        EXPECT_EQ(device_keys.algorithms[1], "m.megolm.v1.aes-sha2");
        EXPECT_EQ(device_keys.keys.size(), 2);
        EXPECT_EQ(device_keys.keys["curve25519:JLAFKJWSCS"],
                  "3C5BFWi2Y8MaVvjM8M22DBmh24PmgR0nPvJOIArzgyI");
        EXPECT_EQ(device_keys.keys["ed25519:JLAFKJWSCS"],
                  "lEuiRJBit0IG6nUf5pUzWTUEsRVVe/HJkoKuEww9ULI");
        EXPECT_EQ(
          device_keys.signatures["@alice:example.com"]["ed25519:JLAFKJWSCS"],
          "dSO80A01XiigH3uBiDVx/EjzaoycHcjq9lfQX0uWsqxl2giMIiSPR8a4d291W1ihKJL/a+myXS367WT6NAIcBA");
        EXPECT_EQ(device_keys.unsigned_info.device_display_name, "Alice's mobile phone");

        auto master_keys       = res.master_keys["@alice:example.org"];
        auto self_signing_keys = res.self_signing_keys["@alice:example.org"];
        auto user_signing_keys = res.user_signing_keys["@alice:example.org"];

        EXPECT_EQ(master_keys.keys.size(), 1);
        EXPECT_EQ(self_signing_keys.keys.size(), 1);
        EXPECT_EQ(user_signing_keys.keys.size(), 1);
        EXPECT_EQ(master_keys.signatures["@alice:example.org"].size(), 1);
        EXPECT_EQ(self_signing_keys.signatures["@alice:example.org"].size(), 1);
        EXPECT_EQ(user_signing_keys.signatures["@alice:example.org"].size(), 1);
}

TEST(Crypto, KeyChanges)
{
        json data = R"({
          "changed": [
            "@alice:example.com",
            "@bob:example.org"
          ],
          "left": [
            "@clara:example.com",
            "@doug:example.org"
          ]
        })"_json;

        KeyChanges res = data;

        EXPECT_EQ(res.changed.size(), 2);
        EXPECT_EQ(res.changed[0], "@alice:example.com");
        EXPECT_EQ(res.changed[1], "@bob:example.org");

        EXPECT_EQ(res.left.size(), 2);
        EXPECT_EQ(res.left[0], "@clara:example.com");
        EXPECT_EQ(res.left[1], "@doug:example.org");
}

TEST(Crypto, ClaimKeys)
{
        json data = R"({
          "failures": {},
          "one_time_keys": {
            "@alice:example.com": {
              "JLAFKJWSCS": {
                "signed_curve25519:AAAAHg": {
                  "key": "zKbLg+NrIjpnagy+pIY6uPL4ZwEG2v+8F9lmgsnlZzs",
                  "signatures": {
                    "@alice:example.com": {
                      "ed25519:JLAFKJWSCS": "FLWxXqGbwrb8SM3Y795eB6OA8bwBcoMZFXBqnTn58AYWZSqiD45tlBVcDa2L7RwdKXebW/VzDlnfVJ+9jok1Bw"
                    }
		  }
                }
	      }
	    }
	  }
	})"_json;

        ClaimKeys res = data;
        EXPECT_EQ(res.failures.size(), 0);
        EXPECT_EQ(res.one_time_keys.size(), 1);

        auto device = res.one_time_keys["@alice:example.com"]["JLAFKJWSCS"];
        EXPECT_EQ(device["signed_curve25519:AAAAHg"]["key"],
                  "zKbLg+NrIjpnagy+pIY6uPL4ZwEG2v+8F9lmgsnlZzs");
}

TEST(Responses, Notifications)
{
        json data = R"({
          "next_token": "abcdef",
          "notifications": [{
            "actions": [
              "notify"
            ],
          "profile_tag": null,
          "read": true,
          "room_id": "!abcdefg:example.com",
          "ts": 1475508881945,
          "event": {
            "sender": "@alice:example.com",
            "type": "m.room.message",
            "age": 124524,
            "txn_id": "1234",
            "content": {
              "body": "I am a fish",
              "msgtype": "m.text"
            },
            "origin_server_ts": 1417731086797,
            "event_id": "$74686972643033:example.com"
           }
         }]
	})"_json;

        mtx::responses::Notifications notif = data;

        // EXPECT_EQ(notif.next_token, "abcdef");
        EXPECT_EQ(notif.notifications.size(), 1);
        EXPECT_EQ(notif.notifications.at(0).profile_tag, "");
        EXPECT_EQ(notif.notifications.at(0).read, true);
        EXPECT_EQ(notif.notifications.at(0).ts, 1475508881945L);
        EXPECT_EQ(notif.notifications.at(0).room_id, "!abcdefg:example.com");

        using TextEvent = mtx::events::RoomEvent<msg::Text>;
        auto event      = std::get<TextEvent>(notif.notifications.at(0).event);

        EXPECT_EQ(event.content.body, "I am a fish");
        EXPECT_EQ(event.sender, "@alice:example.com");
}

TEST(Responses, Userinteractive)
{
        json data =
          R"(
{
  "completed": [ "example.type.foo" ],
  "session": "YQVPFRiztSYtmsjLNQmsxTCg",
  "flows": [
    {
      "stages": [
        "m.login.recaptcha",
        "m.login.terms",
        "m.login.dummy"
      ]
    },
    {
      "stages": [
        "m.login.recaptcha",
        "m.login.terms",
        "m.login.email.identity"
      ]
    }
  ],
  "params": {
    "m.login.recaptcha": {
      "public_key": "6LcgI54UAAAAABGdGmruw    6DdOocFpYVdjYBRe4zb"
    },
    "m.login.terms": {
      "policies": {
        "privacy_policy": {
          "version": "1.0",
          "en": {
            "name": "Terms and Conditions",
            "url": "https://matrix-client.matrix.org/_matrix/consent?v=1.0"
          }
        }
      }
    }
  }
})"_json;
        mtx::user_interactive::Unauthorized unauthorized = data;

        EXPECT_EQ(unauthorized.completed[0], "example.type.foo");
        EXPECT_EQ(unauthorized.session, "YQVPFRiztSYtmsjLNQmsxTCg");
        EXPECT_EQ(unauthorized.flows.size(), 2);
        EXPECT_EQ(unauthorized.flows[0].stages[0], "m.login.recaptcha");
        EXPECT_EQ(unauthorized.flows[0].stages[1], "m.login.terms");
        EXPECT_EQ(unauthorized.flows[0].stages[2], "m.login.dummy");
        EXPECT_EQ(unauthorized.flows[1].stages[0], "m.login.recaptcha");
        EXPECT_EQ(unauthorized.flows[1].stages[1], "m.login.terms");
        EXPECT_EQ(unauthorized.flows[1].stages[2], "m.login.email.identity");

        EXPECT_EQ(std::get<mtx::user_interactive::TermsParams>(
                    unauthorized.params[std::string{mtx::user_interactive::auth_types::terms}])
                    .policies.size(),
                  1);
        EXPECT_EQ(std::get<mtx::user_interactive::TermsParams>(
                    unauthorized.params[std::string{mtx::user_interactive::auth_types::terms}])
                    .policies["privacy_policy"]
                    .version,
                  "1.0");
        EXPECT_EQ(std::get<mtx::user_interactive::TermsParams>(
                    unauthorized.params[std::string{mtx::user_interactive::auth_types::terms}])
                    .policies["privacy_policy"]
                    .langToPolicy["en"]
                    .name,
                  "Terms and Conditions");

        json data2 = R"(
{
  "session": "CFNYzCbLYyGTpURjdmkIXMHc",
  "flows": [
    {
      "stages": [
        "m.login.password"
      ]
    }
  ],
  "params": {}
})"_json;

        unauthorized = data2;
        EXPECT_EQ(unauthorized.flows[0].stages[0], mtx::user_interactive::auth_types::password);
}

TEST(Responses, TurnServer)
{
        json data = R"({
          "username": "1443779631:@user:example.com",
          "password": "JlKfBy1QwLrO20385QyAtEyIv0=",
          "uris": [
            "turn:turn.example.com:3478?transport=udp",
            "turn:10.20.30.40:3478?transport=tcp",
            "turns:10.20.30.40:443?transport=tcp"
          ],
          "ttl": 86400
        })"_json;

        TurnServer turnServer = data;
        EXPECT_EQ(turnServer.username, "1443779631:@user:example.com");
        EXPECT_EQ(turnServer.password, "JlKfBy1QwLrO20385QyAtEyIv0=");
        EXPECT_EQ(turnServer.uris[0], "turn:turn.example.com:3478?transport=udp");
        EXPECT_EQ(turnServer.uris[1], "turn:10.20.30.40:3478?transport=tcp");
        EXPECT_EQ(turnServer.uris[2], "turns:10.20.30.40:443?transport=tcp");
        EXPECT_EQ(turnServer.ttl, 86400);
}

TEST(Responses, PublicRoomVisibility)
{
        json data                                           = {{"visibility", "public"}};
        mtx::responses::PublicRoomVisibility roomVisibility = data;
        EXPECT_EQ(roomVisibility.visibility, mtx::common::RoomVisibility::Public);

        data           = {{"visibility", "private"}};
        roomVisibility = data;
        EXPECT_EQ(roomVisibility.visibility, mtx::common::RoomVisibility::Private);
}

TEST(Responses, PublicRooms)
{
        json data = R"({
          "chunk": [
            {
              "aliases": [
                "#murrays:cheese.bar"
              ],
              "avatar_url": "mxc://bleeker.street/CHEDDARandBRIE",
              "guest_can_join": false,
              "name": "CHEESE",
              "num_joined_members": 37,
              "room_id": "!ol19s:bleecker.street",
              "topic": "Tasty tasty cheese",
              "world_readable": true
            }
          ],
          "next_batch": "p190q",
          "prev_batch": "p1902",
          "total_room_count_estimate": 115
        })"_json;

        PublicRooms publicRooms = data;
        EXPECT_EQ(publicRooms.chunk.size(), 1);
        auto &chunk = publicRooms.chunk[0];
        EXPECT_EQ(chunk.aliases.size(), 1);
        EXPECT_EQ(chunk.aliases[0], "#murrays:cheese.bar");
        EXPECT_EQ(chunk.avatar_url, "mxc://bleeker.street/CHEDDARandBRIE");
        EXPECT_EQ(chunk.guest_can_join, false);
        EXPECT_EQ(chunk.name, "CHEESE");
        EXPECT_EQ(chunk.num_joined_members, 37);
        EXPECT_EQ(chunk.room_id, "!ol19s:bleecker.street");
        EXPECT_EQ(chunk.topic, "Tasty tasty cheese");
        EXPECT_EQ(publicRooms.next_batch, "p190q");
        EXPECT_EQ(publicRooms.prev_batch, "p1902");
        EXPECT_EQ(publicRooms.total_room_count_estimate, 115);
}
