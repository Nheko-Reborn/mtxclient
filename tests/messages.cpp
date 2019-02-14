#include <gtest/gtest.h>

#include <nlohmann/json.hpp>
#include <mtx.hpp>

using json = nlohmann::json;

using namespace mtx::events;

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

        RoomEvent<msg::Redacted> event = data;

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
              "url": "mxc://localhost/ffed755USFFxlgbQYZGtryd"
          },
          "event_id": "$143273582443PhrSn:localhost",
          "origin_server_ts": 1432735824653,
          "room_id": "!jEsUZKDJdhlrceRyVU:localhost",
          "sender": "@example:localhost",
          "type": "m.room.message"
        })"_json;

        RoomEvent<msg::Audio> event = data;

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
            "msgtype": "m.emote"
          },
          "type": "m.room.message",
          "room_id": "!VaMCVKSVcyPtXbcMXh:matrix.org"
        })"_json;

        RoomEvent<msg::Emote> event = data;

        EXPECT_EQ(event.type, EventType::RoomMessage);
        EXPECT_EQ(event.event_id, "$15098786822025533uttji:matrix.org");
        EXPECT_EQ(event.room_id, "!VaMCVKSVcyPtXbcMXh:matrix.org");
        EXPECT_EQ(event.sender, "@mujx:matrix.org");
        EXPECT_EQ(event.origin_server_ts, 1509878682149L);
        EXPECT_EQ(event.unsigned_data.age, 626351821);
        EXPECT_EQ(event.content.body, "tests");
        EXPECT_EQ(event.content.msgtype, "m.emote");
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
              "size": 40565
            },
            "msgtype": "m.file",
            "url": "mxc://matrix.org/XpxykZBESCSQnYkLKbbIKnVn"
          },
          "type": "m.room.message",
          "room_id": "!lfoDRlNFWlvOnvkBwQ:matrix.org"
        })"_json;

        RoomEvent<msg::File> event = data;

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
            "url": "mxc://kamax.io/ewDDLHYnysbHYCPViZwAEIjT"
          },
          "type": "m.room.message",
          "room_id": "!cURbafjkfsMDVwdRDQ:matrix.org"
        })"_json;

        RoomEvent<msg::Image> event = data;

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
            "msgtype": "m.notice"
          },
          "type": "m.room.message",
          "room_id": "!BPvgRcBVHzyFSlYkrg:matrix.org"
        })"_json;

        RoomEvent<msg::Notice> event = data;

        EXPECT_EQ(event.type, EventType::RoomMessage);
        EXPECT_EQ(event.event_id, "$15104358652239178iCnZy:matrix.org");
        EXPECT_EQ(event.room_id, "!BPvgRcBVHzyFSlYkrg:matrix.org");
        EXPECT_EQ(event.sender, "@_neb_github:matrix.org");
        EXPECT_EQ(event.origin_server_ts, 1510435865515L);
        EXPECT_EQ(event.unsigned_data.age, 69168455);
        EXPECT_EQ(event.content.body,
                  "https://github.com/postmarketOS/pmbootstrap/issues/900 : Package nheko");
        EXPECT_EQ(event.content.msgtype, "m.notice");
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
            "msgtype": "m.text"
          },
          "type": "m.room.message",
          "room_id": "!lfoDRlNFWlvOnvkBwQ:matrix.org"
         })"_json;

        RoomEvent<msg::Text> event = data;

        EXPECT_EQ(event.type, EventType::RoomMessage);
        EXPECT_EQ(event.event_id, "$15104893562785758wEgEU:matrix.org");
        EXPECT_EQ(event.room_id, "!lfoDRlNFWlvOnvkBwQ:matrix.org");
        EXPECT_EQ(event.sender, "@nheko_test:matrix.org");
        EXPECT_EQ(event.origin_server_ts, 1510489356530L);
        EXPECT_EQ(event.unsigned_data.age, 2225);
        EXPECT_EQ(event.unsigned_data.transaction_id, "m1510489356267.2");

        EXPECT_EQ(event.content.body, "hey there");
        EXPECT_EQ(event.content.msgtype, "m.text");
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
              "url": "mxc://localhost/ffed755USFFxlgbQYZGtryd"
          },
          "event_id": "$143273582443PhrSn:localhost",
          "origin_server_ts": 1432735824653,
          "room_id": "!jEsUZKDJdhlrceRyVU:localhost",
          "sender": "@example:localhost",
          "type": "m.room.message"
        })"_json;

        RoomEvent<msg::Video> event = data;

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
            "url": "mxc://matrix.org/sHhqkFCvSkFwtmvtETOtKnLP"
          },
          "event_id": "$WLGTSEFSEF:localhost",
          "origin_server_ts": 1431961217939,
          "room_id": "!Cuyf34gef24t:localhost",
          "sender": "@example:localhost",
          "type": "m.sticker"
	})"_json;

        Sticker event = data;

        EXPECT_EQ(event.type, EventType::Sticker);
        EXPECT_EQ(event.event_id, "$WLGTSEFSEF:localhost");
        EXPECT_EQ(event.content.body, "Landing");
        EXPECT_EQ(event.content.url, "mxc://matrix.org/sHhqkFCvSkFwtmvtETOtKnLP");
        EXPECT_EQ(event.content.info.w, 140);
        EXPECT_EQ(event.content.info.h, 200);
        EXPECT_EQ(event.content.info.size, 73602);
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

        RoomEvent<msg::Notice> notice = notice_data;

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

        RoomEvent<msg::Text> text = text_data;

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

        RoomEvent<msg::Emote> emote = emote_data;

        EXPECT_EQ(emote.content.msgtype, "m.emote");
        EXPECT_EQ(emote.content.format, "org.matrix.custom.html");
        EXPECT_EQ(emote.content.formatted_body, "<h1> Hello World! </h1>");
}
