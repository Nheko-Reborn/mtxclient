#include <gtest/gtest.h>

#include <mtx.hpp>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

using namespace mtx::events;

TEST(RoomEvents, Reaction)
{
    json data = R"({
  "type": "m.reaction",
  "room_id": "!CYvyeleADEeDAsndMom:localhost",
  "sender": "@example:localhost",
  "content": {
    "m.relates_to": {
      "rel_type": "m.annotation",
      "event_id": "$oGKg0tfsnDamWPsGxUptGLWR5b8Xq6QNFFsysQNSnake",
      "key": "👀"
    }
  },
  "origin_server_ts": 1588536414112,
  "unsigned": {
    "age": 1905609
  },
  "event_id": "$ujXAq1WXebS-vcpA4yBIZPvCeqGvnrMFP1c1qn8_wJump"
  })"_json;

    RoomEvent<msg::Reaction> event = data.get<RoomEvent<msg::Reaction>>();

    EXPECT_EQ(event.type, EventType::Reaction);
    EXPECT_EQ(event.event_id, "$ujXAq1WXebS-vcpA4yBIZPvCeqGvnrMFP1c1qn8_wJump");
    EXPECT_EQ(event.room_id, "!CYvyeleADEeDAsndMom:localhost");
    EXPECT_EQ(event.sender, "@example:localhost");
    EXPECT_EQ(event.origin_server_ts, 1588536414112L);
    EXPECT_EQ(event.unsigned_data.age, 1905609L);
    EXPECT_EQ(event.content.relations.relations.at(0).event_id,
              "$oGKg0tfsnDamWPsGxUptGLWR5b8Xq6QNFFsysQNSnake");
    EXPECT_EQ(event.content.relations.relations.at(0).key, "👀");
    EXPECT_EQ(event.content.relations.relations.at(0).rel_type,
              mtx::common::RelationType::Annotation);

    EXPECT_EQ(data.dump(), json(event).dump());
}

TEST(RoomEvents, Redacted)
{
    json data = R"({
	  "unsigned": {
            "age": 146,
	    "redacted_by": "$152135702813129HltcO:matrix.org"
	  },
          "content": {},
          "event_id": "$143273582443PhrSn:localhost",
          "origin_server_ts": 1432735824653,
          "room_id": "!jEsUZKDJdhlrceRyVU:localhost",
          "sender": "@example:localhost",
          "type": "m.room.message"
        })"_json;

    RoomEvent<msg::Redacted> event = data.get<RoomEvent<msg::Redacted>>();

    EXPECT_EQ(event.type, EventType::RoomMessage);
    EXPECT_EQ(event.event_id, "$143273582443PhrSn:localhost");
    EXPECT_EQ(event.room_id, "!jEsUZKDJdhlrceRyVU:localhost");
    EXPECT_EQ(event.sender, "@example:localhost");
    EXPECT_EQ(event.origin_server_ts, 1432735824653L);
    EXPECT_EQ(event.unsigned_data.age, 146);
    EXPECT_EQ(event.unsigned_data.redacted_by, "$152135702813129HltcO:matrix.org");
}

TEST(RoomEvents, AudioMessage)
{
    json data = R"({
	  "unsigned": {
            "age": 146
	  },
          "content": {
              "body": "Bee Gees - Stayin' Alive",
              "info": {
                  "duration": 2140786,
                  "mimetype": "audio/mpeg",
                  "size": 1563685
              },
              "msgtype": "m.audio",
              "url": "mxc://localhost/ffed755USFFxlgbQYZGtryd",
	      "m.relates_to": {
		  "m.in_reply_to": {
                       "event_id": "$6GKhAfJOcwNd69lgSizdcTob8z2pWQgBOZPrnsWMA1E"
                  }
              }
          },
          "event_id": "$143273582443PhrSn:localhost",
          "origin_server_ts": 1432735824653,
          "room_id": "!jEsUZKDJdhlrceRyVU:localhost",
          "sender": "@example:localhost",
          "type": "m.room.message"
        })"_json;

    RoomEvent<msg::Audio> event = data.get<RoomEvent<msg::Audio>>();

    EXPECT_EQ(event.type, EventType::RoomMessage);
    EXPECT_EQ(event.event_id, "$143273582443PhrSn:localhost");
    EXPECT_EQ(event.room_id, "!jEsUZKDJdhlrceRyVU:localhost");
    EXPECT_EQ(event.sender, "@example:localhost");
    EXPECT_EQ(event.origin_server_ts, 1432735824653L);
    EXPECT_EQ(event.unsigned_data.age, 146);

    EXPECT_EQ(event.content.body, "Bee Gees - Stayin' Alive");
    EXPECT_EQ(event.content.msgtype, "m.audio");
    EXPECT_EQ(event.content.url, "mxc://localhost/ffed755USFFxlgbQYZGtryd");
    EXPECT_EQ(event.content.info.mimetype, "audio/mpeg");
    EXPECT_EQ(event.content.info.size, 1563685);
    EXPECT_EQ(event.content.info.duration, 2140786);
    EXPECT_EQ(event.content.relations.reply_to().value(),
              "$6GKhAfJOcwNd69lgSizdcTob8z2pWQgBOZPrnsWMA1E");
    EXPECT_EQ(event.content.relations.relations.at(0).rel_type,
              mtx::common::RelationType::InReplyTo);
}

TEST(RoomEvents, ConfettiMessage)
{
    json data = R"({
          "origin_server_ts": 1510489356530,
          "sender": "@nheko_test:matrix.org",
          "event_id": "$15104893562785758wEgEU:matrix.org",
          "unsigned": {
            "age": 2225,
            "transaction_id": "m1510489356267.2"
          },
          "content": {
            "body": "party!",
            "msgtype": "nic.custom.confetti",
	      "m.relates_to": {
		  "m.in_reply_to": {
                       "event_id": "$6GKhAfJOcwNd69lgSizdcTob8z2pWQgBOZPrnsWMA1E"
                  }
              }
          },
          "type": "m.room.message",
          "room_id": "!lfoDRlNFWlvOnvkBwQ:matrix.org"
         })"_json;

    RoomEvent<msg::Confetti> event = data.get<RoomEvent<msg::Confetti>>();

    EXPECT_EQ(event.type, EventType::RoomMessage);
    EXPECT_EQ(event.event_id, "$15104893562785758wEgEU:matrix.org");
    EXPECT_EQ(event.room_id, "!lfoDRlNFWlvOnvkBwQ:matrix.org");
    EXPECT_EQ(event.sender, "@nheko_test:matrix.org");
    EXPECT_EQ(event.origin_server_ts, 1510489356530L);
    EXPECT_EQ(event.unsigned_data.age, 2225);
    EXPECT_EQ(event.unsigned_data.transaction_id, "m1510489356267.2");

    EXPECT_EQ(event.content.body, "party!");
    EXPECT_EQ(event.content.msgtype, "nic.custom.confetti");
    EXPECT_EQ(event.content.relations.relations.at(0).event_id,
              "$6GKhAfJOcwNd69lgSizdcTob8z2pWQgBOZPrnsWMA1E");
    EXPECT_EQ(event.content.relations.relations.at(0).rel_type,
              mtx::common::RelationType::InReplyTo);

    EXPECT_EQ(data.dump(), json(event).dump());
}

TEST(RoomEvents, EmoteMessage)
{
    json data = R"({
          "origin_server_ts": 1509878682149,
          "sender": "@mujx:matrix.org",
          "event_id": "$15098786822025533uttji:matrix.org",
          "unsigned": {
            "age": 626351821
          },
          "content": {
            "body": "tests",
            "msgtype": "m.emote",
	      "m.relates_to": {
		  "m.in_reply_to": {
                       "event_id": "$6GKhAfJOcwNd69lgSizdcTob8z2pWQgBOZPrnsWMA1E"
                  }
              }
          },
          "type": "m.room.message",
          "room_id": "!VaMCVKSVcyPtXbcMXh:matrix.org"
        })"_json;

    RoomEvent<msg::Emote> event = data.get<RoomEvent<msg::Emote>>();

    EXPECT_EQ(event.type, EventType::RoomMessage);
    EXPECT_EQ(event.event_id, "$15098786822025533uttji:matrix.org");
    EXPECT_EQ(event.room_id, "!VaMCVKSVcyPtXbcMXh:matrix.org");
    EXPECT_EQ(event.sender, "@mujx:matrix.org");
    EXPECT_EQ(event.origin_server_ts, 1509878682149L);
    EXPECT_EQ(event.unsigned_data.age, 626351821);
    EXPECT_EQ(event.content.body, "tests");
    EXPECT_EQ(event.content.msgtype, "m.emote");
    EXPECT_EQ(event.content.relations.relations.at(0).event_id,
              "$6GKhAfJOcwNd69lgSizdcTob8z2pWQgBOZPrnsWMA1E");
    EXPECT_EQ(event.content.relations.relations.at(0).rel_type,
              mtx::common::RelationType::InReplyTo);
}

TEST(RoomEvents, FileMessage)
{
    json data = R"({
          "origin_server_ts": 1510485607737,
          "sender": "@nheko_test:matrix.org",
          "event_id": "$15104856072749611ERqhw:matrix.org",
          "unsigned": {
            "age": 31,
            "transaction_id": "m1510485607454.1"
          },
          "content": {
            "body": "optimize.pdf",
            "info": {
              "mimetype": "application/pdf",
              "size": 40565,
	      "thumbnail_info": {
                "h": 200,
                "mimetype": "image/png",
                "size": 73602,
                "w": 140
              },
	      "thumbnail_url": "mxc://matrix.org/XpxykZBESCSQnYkLKbbIKnVn"
            },
            "msgtype": "m.file",
            "url": "mxc://matrix.org/XpxykZBESCSQnYkLKbbIKnVn",
	      "m.relates_to": {
		  "m.in_reply_to": {
                       "event_id": "$6GKhAfJOcwNd69lgSizdcTob8z2pWQgBOZPrnsWMA1E"
                  }
              }
          },
          "type": "m.room.message",
          "room_id": "!lfoDRlNFWlvOnvkBwQ:matrix.org"
        })"_json;

    RoomEvent<msg::File> event = data.get<RoomEvent<msg::File>>();

    EXPECT_EQ(event.type, EventType::RoomMessage);
    EXPECT_EQ(event.event_id, "$15104856072749611ERqhw:matrix.org");
    EXPECT_EQ(event.room_id, "!lfoDRlNFWlvOnvkBwQ:matrix.org");
    EXPECT_EQ(event.sender, "@nheko_test:matrix.org");
    EXPECT_EQ(event.origin_server_ts, 1510485607737L);
    EXPECT_EQ(event.unsigned_data.age, 31);
    EXPECT_EQ(event.unsigned_data.transaction_id, "m1510485607454.1");

    EXPECT_EQ(event.content.body, "optimize.pdf");
    EXPECT_EQ(event.content.msgtype, "m.file");
    EXPECT_EQ(event.content.url, "mxc://matrix.org/XpxykZBESCSQnYkLKbbIKnVn");
    EXPECT_EQ(event.content.info.mimetype, "application/pdf");
    EXPECT_EQ(event.content.info.size, 40565);
    EXPECT_EQ(event.content.relations.relations.at(0).event_id,
              "$6GKhAfJOcwNd69lgSizdcTob8z2pWQgBOZPrnsWMA1E");
    EXPECT_EQ(event.content.relations.relations.at(0).rel_type,
              mtx::common::RelationType::InReplyTo);

    json withThumb = event;
    EXPECT_EQ(withThumb["content"]["info"].count("thumbnail_url"), 1);
    EXPECT_EQ(withThumb["content"]["info"].count("thumbnail_info"), 1);

    event.content.info.thumbnail_url = "";
    json withoutThumb                = event;
    EXPECT_EQ(withoutThumb["content"]["info"].count("thumbnail_url"), 0);
    EXPECT_EQ(withoutThumb["content"]["info"].count("thumbnail_info"), 0);
}

TEST(RoomEvents, EncryptedImageMessage)
{
    json data                   = R"(
{
  "content": {
    "body": "something-important.jpg",
    "file": {
      "url": "mxc://example.org/FHyPlCeYUSFFxlgbQYZmoEoe",
      "mimetype": "image/jpeg",
      "v": "v2",
      "key": {
        "alg": "A256CTR",
        "ext": true,
        "k": "aWF6-32KGYaC3A_FEUCk1Bt0JA37zP0wrStgmdCaW-0",
        "key_ops": ["encrypt","decrypt"],
        "kty": "oct"
      },
      "iv": "w+sE15fzSc0AAAAAAAAAAA",
      "hashes": {
        "sha256": "fdSLu/YkRx3Wyh3KQabP3rd6+SFiKg5lsJZQHtkSAYA"
      }
    },
    "info": {
      "mimetype": "image/jpeg",
      "h": 1536,
      "size": 422018,
      "thumbnail_file": {
        "hashes": {
          "sha256": "/NogKqW5bz/m8xHgFiH5haFGjCNVmUIPLzfvOhHdrxY"
        },
        "iv": "U+k7PfwLr6UAAAAAAAAAAA",
        "key": {
          "alg": "A256CTR",
          "ext": true,
          "k": "RMyd6zhlbifsACM1DXkCbioZ2u0SywGljTH8JmGcylg",
          "key_ops": ["encrypt", "decrypt"],
          "kty": "oct"
        },
        "mimetype": "image/jpeg",
        "url": "mxc://example.org/pmVJxyxGlmxHposwVSlOaEOv",
        "v": "v2"
      },
      "thumbnail_info": {
        "h": 768,
        "mimetype": "image/jpeg",
        "size": 211009,
        "w": 432
      },
      "w": 864
    },
    "msgtype": "m.image"
  },
  "event_id": "$143273582443PhrSn:example.org",
  "origin_server_ts": 1432735824653,
  "room_id": "!jEsUZKDJdhlrceRyVU:example.org",
  "sender": "@example:example.org",
  "type": "m.room.message",
  "unsigned": {
      "age": 1234
  }
})"_json;
    RoomEvent<msg::Image> event = data.get<RoomEvent<msg::Image>>();

    EXPECT_EQ(event.type, EventType::RoomMessage);
    EXPECT_EQ(event.event_id, "$143273582443PhrSn:example.org");
    EXPECT_EQ(event.room_id, "!jEsUZKDJdhlrceRyVU:example.org");
    EXPECT_EQ(event.sender, "@example:example.org");
    EXPECT_EQ(event.origin_server_ts, 1432735824653L);
    EXPECT_EQ(event.unsigned_data.age, 1234);

    EXPECT_EQ(event.content.body, "something-important.jpg");
    EXPECT_EQ(event.content.msgtype, "m.image");
    EXPECT_EQ(event.content.url, "");
    EXPECT_EQ(event.content.info.mimetype, "image/jpeg");
    EXPECT_EQ(event.content.info.size, 422018);
    EXPECT_EQ(event.content.file.value().url, "mxc://example.org/FHyPlCeYUSFFxlgbQYZmoEoe");
    EXPECT_EQ(event.content.info.thumbnail_file.value().url,
              "mxc://example.org/pmVJxyxGlmxHposwVSlOaEOv");
}

TEST(RoomEvents, ImageMessage)
{
    json data = R"({
          "origin_server_ts": 1510504294993,
          "sender": "@max:kamax.io",
          "event_id": "$15105042942524OGmZm:kamax.io",
          "unsigned": {
            "age": 738977
          },
          "content": {
            "body": "image.png",
            "info": {
              "mimetype": "image/png",
              "thumbnail_info": {
                "mimetype": "image/png",
                "h": 302,
                "w": 474,
                "size": 33504
              },
              "h": 302,
              "thumbnail_url": "mxc://kamax.io/IlTRDmpGMPkiwlyYUpHXSqjH",
              "w": 474,
              "size": 32573
            },
            "msgtype": "m.image",
            "url": "mxc://kamax.io/ewDDLHYnysbHYCPViZwAEIjT",
	      "m.relates_to": {
		  "m.in_reply_to": {
                       "event_id": "$6GKhAfJOcwNd69lgSizdcTob8z2pWQgBOZPrnsWMA1E"
                  }
              }
          },
          "type": "m.room.message",
          "room_id": "!cURbafjkfsMDVwdRDQ:matrix.org"
        })"_json;

    RoomEvent<msg::Image> event = data.get<RoomEvent<msg::Image>>();

    EXPECT_EQ(event.type, EventType::RoomMessage);
    EXPECT_EQ(event.event_id, "$15105042942524OGmZm:kamax.io");
    EXPECT_EQ(event.room_id, "!cURbafjkfsMDVwdRDQ:matrix.org");
    EXPECT_EQ(event.sender, "@max:kamax.io");
    EXPECT_EQ(event.origin_server_ts, 1510504294993L);
    EXPECT_EQ(event.unsigned_data.age, 738977);

    EXPECT_EQ(event.content.body, "image.png");
    EXPECT_EQ(event.content.info.mimetype, "image/png");
    EXPECT_EQ(event.content.info.h, 302);
    EXPECT_EQ(event.content.info.w, 474);
    EXPECT_EQ(event.content.info.size, 32573);
    EXPECT_EQ(event.content.info.thumbnail_url, "mxc://kamax.io/IlTRDmpGMPkiwlyYUpHXSqjH");
    EXPECT_EQ(event.content.info.thumbnail_info.mimetype, "image/png");
    EXPECT_EQ(event.content.info.thumbnail_info.w, 474);
    EXPECT_EQ(event.content.info.thumbnail_info.h, 302);
    EXPECT_EQ(event.content.info.thumbnail_info.size, 33504);
    EXPECT_EQ(event.content.relations.relations.at(0).event_id,
              "$6GKhAfJOcwNd69lgSizdcTob8z2pWQgBOZPrnsWMA1E");
    EXPECT_EQ(event.content.relations.relations.at(0).rel_type,
              mtx::common::RelationType::InReplyTo);

    json withThumb = event;
    EXPECT_EQ(withThumb["content"]["info"].count("thumbnail_url"), 1);
    EXPECT_EQ(withThumb["content"]["info"].count("thumbnail_info"), 1);

    event.content.info.thumbnail_url = "";
    json withoutThumb                = event;
    EXPECT_EQ(withoutThumb["content"]["info"].count("thumbnail_url"), 0);
    EXPECT_EQ(withoutThumb["content"]["info"].count("thumbnail_info"), 0);

    data = R"({
          "origin_server_ts": 1510504294993,
          "sender": "@max:kamax.io",
          "event_id": "$15105042942524OGmZm:kamax.io",
          "unsigned": {
            "age": 738977
          },
          "content": {
            "body": "image.png",
            "info": {
              "mimetype": "image/png",
              "thumbnail_info": {
                "mimetype": "image/png",
                "h": null,
                "w": null,
                "size": null
              },
              "h": null,
              "thumbnail_url": "mxc://kamax.io/IlTRDmpGMPkiwlyYUpHXSqjH",
              "w": null,
              "size": null
            },
            "msgtype": "m.image",
            "url": "mxc://kamax.io/ewDDLHYnysbHYCPViZwAEIjT",
	      "m.relates_to": {
		  "m.in_reply_to": {
                       "event_id": "$6GKhAfJOcwNd69lgSizdcTob8z2pWQgBOZPrnsWMA1E"
                  }
              }
          },
          "type": "m.room.message",
          "room_id": "!cURbafjkfsMDVwdRDQ:matrix.org"
        })"_json;

    event = data.get<RoomEvent<msg::Image>>();

    EXPECT_EQ(event.type, EventType::RoomMessage);
    EXPECT_EQ(event.event_id, "$15105042942524OGmZm:kamax.io");
    EXPECT_EQ(event.room_id, "!cURbafjkfsMDVwdRDQ:matrix.org");
    EXPECT_EQ(event.sender, "@max:kamax.io");
    EXPECT_EQ(event.origin_server_ts, 1510504294993L);
    EXPECT_EQ(event.unsigned_data.age, 738977);

    EXPECT_EQ(event.content.body, "image.png");
    EXPECT_EQ(event.content.info.mimetype, "image/png");
    EXPECT_EQ(event.content.info.h, 0);
    EXPECT_EQ(event.content.info.w, 0);
    EXPECT_EQ(event.content.info.size, 0);
    EXPECT_EQ(event.content.info.thumbnail_url, "mxc://kamax.io/IlTRDmpGMPkiwlyYUpHXSqjH");
    EXPECT_EQ(event.content.info.thumbnail_info.mimetype, "image/png");
    EXPECT_EQ(event.content.info.thumbnail_info.w, 0);
    EXPECT_EQ(event.content.info.thumbnail_info.h, 0);
    EXPECT_EQ(event.content.info.thumbnail_info.size, 0);
    EXPECT_EQ(event.content.relations.relations.at(0).event_id,
              "$6GKhAfJOcwNd69lgSizdcTob8z2pWQgBOZPrnsWMA1E");
    EXPECT_EQ(event.content.relations.relations.at(0).rel_type,
              mtx::common::RelationType::InReplyTo);
}

TEST(RoomEvents, LocationMessage) {}

TEST(RoomEvents, NoticeMessage)
{
    json data = R"({
          "origin_server_ts": 1510435865515,
          "sender": "@_neb_github:matrix.org",
          "event_id": "$15104358652239178iCnZy:matrix.org",
          "unsigned": {
            "age": 69168455
          },
          "content": {
            "body": "https://github.com/postmarketOS/pmbootstrap/issues/900 : Package nheko",
            "msgtype": "m.notice",
	      "m.relates_to": {
		  "m.in_reply_to": {
                       "event_id": "$6GKhAfJOcwNd69lgSizdcTob8z2pWQgBOZPrnsWMA1E"
                  }
              }
          },
          "type": "m.room.message",
          "room_id": "!BPvgRcBVHzyFSlYkrg:matrix.org"
        })"_json;

    RoomEvent<msg::Notice> event = data.get<RoomEvent<msg::Notice>>();

    EXPECT_EQ(event.type, EventType::RoomMessage);
    EXPECT_EQ(event.event_id, "$15104358652239178iCnZy:matrix.org");
    EXPECT_EQ(event.room_id, "!BPvgRcBVHzyFSlYkrg:matrix.org");
    EXPECT_EQ(event.sender, "@_neb_github:matrix.org");
    EXPECT_EQ(event.origin_server_ts, 1510435865515L);
    EXPECT_EQ(event.unsigned_data.age, 69168455);
    EXPECT_EQ(event.content.body,
              "https://github.com/postmarketOS/pmbootstrap/issues/900 : Package nheko");
    EXPECT_EQ(event.content.msgtype, "m.notice");
    EXPECT_EQ(event.content.relations.relations.at(0).event_id,
              "$6GKhAfJOcwNd69lgSizdcTob8z2pWQgBOZPrnsWMA1E");
    EXPECT_EQ(event.content.relations.relations.at(0).rel_type,
              mtx::common::RelationType::InReplyTo);
}

TEST(RoomEvents, TextMessage)
{
    json data = R"({
          "origin_server_ts": 1510489356530,
          "sender": "@nheko_test:matrix.org",
          "event_id": "$15104893562785758wEgEU:matrix.org",
          "unsigned": {
            "age": 2225,
            "transaction_id": "m1510489356267.2"
          },
          "content": {
            "body": "hey there",
            "msgtype": "m.text",
	      "m.relates_to": {
		  "m.in_reply_to": {
                       "event_id": "$6GKhAfJOcwNd69lgSizdcTob8z2pWQgBOZPrnsWMA1E"
                  }
              }
          },
          "type": "m.room.message",
          "room_id": "!lfoDRlNFWlvOnvkBwQ:matrix.org"
         })"_json;

    RoomEvent<msg::Text> event = data.get<RoomEvent<msg::Text>>();

    EXPECT_EQ(event.type, EventType::RoomMessage);
    EXPECT_EQ(event.event_id, "$15104893562785758wEgEU:matrix.org");
    EXPECT_EQ(event.room_id, "!lfoDRlNFWlvOnvkBwQ:matrix.org");
    EXPECT_EQ(event.sender, "@nheko_test:matrix.org");
    EXPECT_EQ(event.origin_server_ts, 1510489356530L);
    EXPECT_EQ(event.unsigned_data.age, 2225);
    EXPECT_EQ(event.unsigned_data.transaction_id, "m1510489356267.2");

    EXPECT_EQ(event.content.body, "hey there");
    EXPECT_EQ(event.content.msgtype, "m.text");
    EXPECT_EQ(event.content.relations.relations.at(0).event_id,
              "$6GKhAfJOcwNd69lgSizdcTob8z2pWQgBOZPrnsWMA1E");
    EXPECT_EQ(event.content.relations.relations.at(0).rel_type,
              mtx::common::RelationType::InReplyTo);

    EXPECT_EQ(data.dump(), json(event).dump());
}

TEST(RoomEvents, VideoMessage)
{
    json data = R"({
	  "unsigned": {
            "age": 146
	  },
          "content": {
              "body": "Gangnam Style",
              "info": {
                  "duration": 2140786,
                  "mimetype": "video/mp4",
                  "size": 1563685,
		  "h": 320,
		  "w": 480,
		  "thumbnail_url": "mxc://localhost/FHyPlCeYUSFFxlgbQYZmoEoe",
		  "thumbnail_info": {
		    "h": 300,
		    "mimetype": "image/jpeg",
		    "size": 46144,
		    "w": 310
		  }
              },
              "msgtype": "m.video",
              "url": "mxc://localhost/ffed755USFFxlgbQYZGtryd",
	      "m.relates_to": {
		  "m.in_reply_to": {
                       "event_id": "$6GKhAfJOcwNd69lgSizdcTob8z2pWQgBOZPrnsWMA1E"
                  }
              }
          },
          "event_id": "$143273582443PhrSn:localhost",
          "origin_server_ts": 1432735824653,
          "room_id": "!jEsUZKDJdhlrceRyVU:localhost",
          "sender": "@example:localhost",
          "type": "m.room.message"
        })"_json;

    RoomEvent<msg::Video> event = data.get<RoomEvent<msg::Video>>();

    EXPECT_EQ(event.type, EventType::RoomMessage);
    EXPECT_EQ(event.event_id, "$143273582443PhrSn:localhost");
    EXPECT_EQ(event.room_id, "!jEsUZKDJdhlrceRyVU:localhost");
    EXPECT_EQ(event.sender, "@example:localhost");
    EXPECT_EQ(event.origin_server_ts, 1432735824653L);
    EXPECT_EQ(event.unsigned_data.age, 146);

    EXPECT_EQ(event.content.body, "Gangnam Style");
    EXPECT_EQ(event.content.msgtype, "m.video");
    EXPECT_EQ(event.content.url, "mxc://localhost/ffed755USFFxlgbQYZGtryd");
    EXPECT_EQ(event.content.info.mimetype, "video/mp4");
    EXPECT_EQ(event.content.info.w, 480);
    EXPECT_EQ(event.content.info.h, 320);
    EXPECT_EQ(event.content.info.size, 1563685);
    EXPECT_EQ(event.content.info.duration, 2140786);
    EXPECT_EQ(event.content.info.thumbnail_info.h, 300);
    EXPECT_EQ(event.content.info.thumbnail_info.w, 310);
    EXPECT_EQ(event.content.info.thumbnail_info.size, 46144);
    EXPECT_EQ(event.content.relations.relations.at(0).event_id,
              "$6GKhAfJOcwNd69lgSizdcTob8z2pWQgBOZPrnsWMA1E");
    EXPECT_EQ(event.content.relations.relations.at(0).rel_type,
              mtx::common::RelationType::InReplyTo);
}

TEST(RoomEvents, Sticker)
{
    json data = R"({
          "age": 242352,
          "content": {
            "body": "Landing",
            "info": {
              "h": 200,
              "mimetype": "image/png",
	      "size": 73602,
	      "thumbnail_info": {
                "h": 200,
                "mimetype": "image/png",
                "size": 73602,
                "w": 140
              },
              "thumbnail_url": "mxc://matrix.org/sHhqkFCvSkFwtmvtETOtKnLP",
              "w": 140
            },
            "url": "mxc://matrix.org/sHhqkFCvSkFwtmvtETOtKnLP",
	      "m.relates_to": {
		  "m.in_reply_to": {
                       "event_id": "$6GKhAfJOcwNd69lgSizdcTob8z2pWQgBOZPrnsWMA1E"
                  }
              }
          },
          "event_id": "$WLGTSEFSEF:localhost",
          "origin_server_ts": 1431961217939,
          "room_id": "!Cuyf34gef24t:localhost",
          "sender": "@example:localhost",
          "type": "m.sticker"
	})"_json;

    Sticker event = data.get<Sticker>();

    EXPECT_EQ(event.type, EventType::Sticker);
    EXPECT_EQ(event.event_id, "$WLGTSEFSEF:localhost");
    EXPECT_EQ(event.content.body, "Landing");
    EXPECT_EQ(event.content.url, "mxc://matrix.org/sHhqkFCvSkFwtmvtETOtKnLP");
    EXPECT_EQ(event.content.info.w, 140);
    EXPECT_EQ(event.content.info.h, 200);
    EXPECT_EQ(event.content.info.size, 73602);
    EXPECT_EQ(event.content.relations.relations.at(0).event_id,
              "$6GKhAfJOcwNd69lgSizdcTob8z2pWQgBOZPrnsWMA1E");
    EXPECT_EQ(event.content.relations.relations.at(0).rel_type,
              mtx::common::RelationType::InReplyTo);

    json data2     = R"({
	  "type": "m.sticker",
	  "sender": "masked",
	  "content": {
	    "msgtype": "m.sticker",
	    "url": "mxc://devture.com/OCPxxxxxxxxxxxxxxxx",
	    "info": {
	      "mimetype": "image/png",
	      "size": 16901,
	      "h": 240,
	      "w": 240
	    }
	  },
	  "origin_server_ts": 1586985031228,
	  "unsigned": {
	    "age": 38
	  },
	  "event_id": "masked",
	  "room_id": "!masked:devture.com"
	})"_json;
    Sticker event2 = data2.get<Sticker>();
    EXPECT_EQ(event2.type, EventType::Sticker);
    EXPECT_EQ(event2.content.body, "");
    EXPECT_EQ(event2.content.url, "mxc://devture.com/OCPxxxxxxxxxxxxxxxx");
    EXPECT_EQ(event2.content.info.w, 240);
    EXPECT_EQ(event2.content.info.h, 240);
    EXPECT_EQ(event2.content.info.size, 16901);
}

TEST(FormattedMessages, Deserialization)
{
    json notice_data = R"({
          "origin_server_ts": 1510435865515,
          "sender": "@_neb_github:matrix.org",
          "event_id": "$15104358652239178iCnZy:matrix.org",
          "content": {
            "body": "https://github.com/postmarketOS/pmbootstrap/issues/900 : Package nheko",
            "msgtype": "m.notice",
            "format": "org.matrix.custom.html",
            "formatted_body": "<h1> Hello World! </h1>"
          },
          "type": "m.room.message",
          "room_id": "!BPvgRcBVHzyFSlYkrg:matrix.org"
        })"_json;

    RoomEvent<msg::Notice> notice = notice_data.get<RoomEvent<msg::Notice>>();

    EXPECT_EQ(notice.type, EventType::RoomMessage);
    EXPECT_EQ(notice.content.msgtype, "m.notice");
    EXPECT_EQ(notice.content.format, "org.matrix.custom.html");
    EXPECT_EQ(notice.content.formatted_body, "<h1> Hello World! </h1>");

    json text_data = R"({
          "origin_server_ts": 1510489356530,
          "sender": "@nheko_test:matrix.org",
          "event_id": "$15104893562785758wEgEU:matrix.org",
          "content": {
            "body": "hey there",
            "msgtype": "m.text",
            "format": "org.matrix.custom.html",
            "formatted_body": "<h1> Hello World! </h1>"
          },
          "type": "m.room.message",
          "room_id": "!lfoDRlNFWlvOnvkBwQ:matrix.org"
         })"_json;

    RoomEvent<msg::Text> text = text_data.get<RoomEvent<msg::Text>>();

    EXPECT_EQ(text.content.msgtype, "m.text");
    EXPECT_EQ(text.content.format, "org.matrix.custom.html");
    EXPECT_EQ(text.content.formatted_body, "<h1> Hello World! </h1>");

    json emote_data = R"({
          "origin_server_ts": 1509878682149,
          "sender": "@mujx:matrix.org",
          "event_id": "$15098786822025533uttji:matrix.org",
          "content": {
            "body": "tests",
            "msgtype": "m.emote",
            "format": "org.matrix.custom.html",
            "formatted_body": "<h1> Hello World! </h1>"
          },
          "type": "m.room.message",
          "room_id": "!VaMCVKSVcyPtXbcMXh:matrix.org"
        })"_json;

    RoomEvent<msg::Emote> emote = emote_data.get<RoomEvent<msg::Emote>>();

    EXPECT_EQ(emote.content.msgtype, "m.emote");
    EXPECT_EQ(emote.content.format, "org.matrix.custom.html");
    EXPECT_EQ(emote.content.formatted_body, "<h1> Hello World! </h1>");
}

TEST(RoomEvents, Encrypted)
{
    json data = R"({
	    "content": {
		"algorithm": "m.megolm.v1.aes-sha2",
		"ciphertext": "AwgAEnACgAkLmt6qF84IK++J7UDH2Za1YVchHyprqTqsg...",
		"device_id": "RJYKSTBOIE",
		"sender_key": "IlRMeOPX2e0MurIyfWEucYBRVOEEUMrOHqn/8mLqMjA",
		"session_id": "X3lUlvLELLYxeTx4yOVu6UDpasGEVO0Jbu+QFnm0cKQ",
	   "m.relates_to": {
		    "m.in_reply_to": {
                         "event_id": "$6GKhAfJOcwNd69lgSizdcTob8z2pWQgBOZPrnsWMA1E"
                    }
                }
	    },
	    "event_id": "$143273582443PhrSn:example.org",
	    "origin_server_ts": 1432735824653,
	    "room_id": "!jEsUZKDJdhlrceRyVU:example.org",
	    "sender": "@example:example.org",
	    "type": "m.room.encrypted",
	    "unsigned": {
		"age": 1234
	    }
	})"_json;

    EncryptedEvent<msg::Encrypted> event = data.get<EncryptedEvent<msg::Encrypted>>();

    std::cout << "*****" << std::endl;
    std::cout << data.dump(2) << std::endl;
    std::cout << "*****" << std::endl;
    std::cout << json(event).dump(2) << std::endl;
    std::cout << "*****" << std::endl;

    EXPECT_EQ(event.type, EventType::RoomEncrypted);
    EXPECT_EQ(event.event_id, "$143273582443PhrSn:example.org");
    EXPECT_EQ(event.room_id, "!jEsUZKDJdhlrceRyVU:example.org");
    EXPECT_EQ(event.sender, "@example:example.org");
    EXPECT_EQ(event.origin_server_ts, 1432735824653L);
    EXPECT_EQ(event.unsigned_data.age, 1234);
    EXPECT_EQ(event.content.algorithm, "m.megolm.v1.aes-sha2");
    EXPECT_EQ(event.content.ciphertext, "AwgAEnACgAkLmt6qF84IK++J7UDH2Za1YVchHyprqTqsg...");
    EXPECT_EQ(event.content.device_id, "RJYKSTBOIE");
    EXPECT_EQ(event.content.sender_key, "IlRMeOPX2e0MurIyfWEucYBRVOEEUMrOHqn/8mLqMjA");
    EXPECT_EQ(event.content.session_id, "X3lUlvLELLYxeTx4yOVu6UDpasGEVO0Jbu+QFnm0cKQ");
    EXPECT_EQ(event.content.relations.relations.at(0).event_id,
              "$6GKhAfJOcwNd69lgSizdcTob8z2pWQgBOZPrnsWMA1E");
    EXPECT_EQ(event.content.relations.relations.at(0).rel_type,
              mtx::common::RelationType::InReplyTo);

    EXPECT_EQ(data, json(event));
}

TEST(RoomEvents, ThreadedMessage)
{
    json data = R"({
          "origin_server_ts": 1510489356530,
          "sender": "@nheko_test:matrix.org",
          "event_id": "$15104893562785758wEgEU:matrix.org",
          "unsigned": {
            "age": 2225,
            "transaction_id": "m1510489356267.2"
          },
          "content": {
            "body": "hey there",
            "msgtype": "m.text",
"m.relates_to": {
  "rel_type": "m.thread",
  "event_id": "$root",
  "m.in_reply_to": {
    "event_id": "$target"
  },
  "is_falling_back": true
}
          },
          "type": "m.room.message",
          "room_id": "!lfoDRlNFWlvOnvkBwQ:matrix.org"
         })"_json;

    RoomEvent<msg::Text> event = data.get<RoomEvent<msg::Text>>();

    EXPECT_EQ(event.type, EventType::RoomMessage);
    EXPECT_EQ(event.event_id, "$15104893562785758wEgEU:matrix.org");
    EXPECT_EQ(event.room_id, "!lfoDRlNFWlvOnvkBwQ:matrix.org");
    EXPECT_EQ(event.sender, "@nheko_test:matrix.org");
    EXPECT_EQ(event.origin_server_ts, 1510489356530L);
    EXPECT_EQ(event.unsigned_data.age, 2225);
    EXPECT_EQ(event.unsigned_data.transaction_id, "m1510489356267.2");

    EXPECT_EQ(event.content.body, "hey there");
    EXPECT_EQ(event.content.msgtype, "m.text");
    EXPECT_EQ(event.content.relations.reply_to(), "$target");
    EXPECT_EQ(event.content.relations.reply_to(false), std::nullopt);
    EXPECT_EQ(event.content.relations.thread(), "$root");

    EXPECT_EQ(data.dump(), json(event).dump());
}
