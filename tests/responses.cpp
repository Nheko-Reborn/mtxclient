#include <gtest/gtest.h>

#include <fstream>

#include <boost/variant.hpp>
#include <nlohmann/json.hpp>

#include <mtx.hpp>

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

        auto aliases = boost::get<StateEvent<state::Aliases>>(state.events[0]);
        EXPECT_EQ(aliases.event_id, "$WLGTSEFSEF:localhost");
        EXPECT_EQ(aliases.type, EventType::RoomAliases);
        EXPECT_EQ(aliases.sender, "@example:localhost");
        EXPECT_EQ(aliases.content.aliases.size(), 2);
        EXPECT_EQ(aliases.content.aliases[0], "#somewhere:localhost");

        auto name = boost::get<StateEvent<state::Name>>(state.events[1]);
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

        name = boost::get<StateEvent<state::Name>>(malformed_state.events[0]);
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
        EXPECT_EQ(room1.ephemeral.receipts.size(), 1);
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
        EXPECT_EQ(room2.ephemeral.receipts.size(), 0);
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

        auto name = boost::get<StrippedEvent<state::Name>>(room.invite_state[0]);
        EXPECT_EQ(name.type, EventType::RoomName);
        EXPECT_EQ(name.content.name, "Testing room");

        auto avatar = boost::get<StrippedEvent<state::Avatar>>(room.invite_state[1]);
        EXPECT_EQ(avatar.type, EventType::RoomAvatar);
        EXPECT_EQ(avatar.content.url, "mxc://matrix.org/wdjzHdrThpqWyVArfyWmRbBx");
}

TEST(Responses, Sync)
{
        std::ifstream file("./fixtures/responses/sync.json");

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
        EXPECT_EQ(nheko.account_data.events.size(), 1);

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
        std::ifstream file("./fixtures/responses/sync_with_crypto.json");

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
                if (boost::get<StateEvent<mtx::events::state::Encryption>>(&e) != nullptr) {
                        auto enc_event  = boost::get<StateEvent<mtx::events::state::Encryption>>(e);
                        algorithm_found = enc_event.content.algorithm;
                        event_id        = enc_event.event_id;
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
          "device_id": "GHTYAJCE"
        })"_json;

        Login login = data;
        EXPECT_EQ(login.user_id.to_string(), "@cheeky_monkey:matrix.org");
        EXPECT_EQ(login.access_token, "abc123");
        EXPECT_EQ(login.home_server, "matrix.org");
        EXPECT_EQ(login.device_id, "GHTYAJCE");

        json data2 = R"({
          "user_id": "@cheeky_monkey:matrix.org",
          "access_token": "abc123", 
	  "home_server": "matrix.org"
        })"_json;

        Login login2 = data2;
        EXPECT_EQ(login2.user_id.to_string(), "@cheeky_monkey:matrix.org");
        EXPECT_EQ(login2.access_token, "abc123");
        EXPECT_EQ(login2.home_server, "matrix.org");
        EXPECT_EQ(login2.device_id, "");
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

        auto first_event = boost::get<RoomEvent<Text>>(messages.chunk[0]);
        EXPECT_EQ(first_event.content.body, "hello world");
        EXPECT_EQ(first_event.content.msgtype, "m.text");
        EXPECT_EQ(first_event.type, mtx::events::EventType::RoomMessage);
        EXPECT_EQ(first_event.event_id, "$1444812213350496Caaaa:example.com");

        auto second_event = boost::get<RoomEvent<Text>>(messages.chunk[1]);
        EXPECT_EQ(second_event.content.body, "the world is big");
        EXPECT_EQ(second_event.content.msgtype, "m.text");
        EXPECT_EQ(second_event.type, mtx::events::EventType::RoomMessage);
        EXPECT_EQ(second_event.event_id, "$1444812213350496Cbbbb:example.com");

        auto third_event = boost::get<StateEvent<Name>>(messages.chunk[2]);
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

        third_event = boost::get<StateEvent<Name>>(messages.chunk[0]);
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

        EXPECT_EQ(ephemeral.typing.size(), 2);
        EXPECT_EQ(ephemeral.typing[0], "@alice:example.com");
        EXPECT_EQ(ephemeral.typing[1], "@bob:example.com");
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

        EXPECT_EQ(ephemeral.typing.size(), 2);
        EXPECT_EQ(ephemeral.typing[0], "@alice:example.com");
        EXPECT_EQ(ephemeral.typing[1], "@bob:example.com");

        EXPECT_EQ(ephemeral.receipts.size(), 5);
        EXPECT_EQ(ephemeral.receipts["$149339947230ohuCC:krtdex.com"].size(), 1);
        EXPECT_EQ(ephemeral.receipts["$14935874261161012PaoJD:matrix.org"].size(), 7);
        EXPECT_EQ(ephemeral.receipts["$14935874261161012PaoJD:matrix.org"]["@matthew:matrix.org"],
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

TEST(Responses, Register)
{
        json data = R"({
          "flows": [{
              "stages": [
                  "m.login.recaptcha"
              ]
          }, {
              "stages": [
                  "m.login.email.identity",
                  "m.login.recaptcha"
              ]
          }
         ],
         "params": {
             "m.login.recaptcha": {
                 "public_key": "6Le31_kSAAAAAK-54VKccKamtr-MFA_3WS1d_fGV"
             }
         },
         "session": "kLmDGyIaqzgCeLgzVEebtNig"
        })"_json;

        RegistrationFlows res = data;

        EXPECT_EQ(res.session, "kLmDGyIaqzgCeLgzVEebtNig");
        EXPECT_EQ(res.flows.size(), 2);

        auto s1 = res.flows.at(0);
        EXPECT_EQ(s1.stages.size(), 1);
        EXPECT_EQ(s1.stages.at(0), "m.login.recaptcha");

        auto s2 = res.flows.at(1);
        EXPECT_EQ(s2.stages.size(), 2);
        EXPECT_EQ(s2.stages.at(0), "m.login.email.identity");
        EXPECT_EQ(s2.stages.at(1), "m.login.recaptcha");
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
        auto event      = boost::get<TextEvent>(notif.notifications.at(0).event);

        EXPECT_EQ(event.content.body, "I am a fish");
        EXPECT_EQ(event.sender, "@alice:example.com");
}
