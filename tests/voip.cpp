#include <gtest/gtest.h>

#include <mtx.hpp>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

using namespace mtx::events;

//! Tests for VoIP v0 support
TEST(VoIPEventsV0, CallInvite)
{
    nlohmann::json data = R"({
          "content": {
            "call_id": "c1591052749788",
            "offer": {
              "sdp": "v=0\r\no=- 6584580628695956864 2 IN IP4 127.0.0.1[...]",
              "type": "offer"
            },
            "version": 0,
            "lifetime": 120000
          },
          "event_id": "$143273582443PhrSn:example.org",
          "origin_server_ts": 1432735824653,
          "room_id": "!jEsUZKDJdhlrceRyVU:example.org",
          "sender": "@example:example.org",
          "type": "m.call.invite",
          "unsigned": {
            "age": 1234
          }
        })"_json;

    RoomEvent<voip::CallInvite> event = data.get<RoomEvent<voip::CallInvite>>();

    EXPECT_EQ(event.event_id, "$143273582443PhrSn:example.org") << "event id";
    EXPECT_EQ(event.origin_server_ts, 1432735824653L) << "origin server ts";
    EXPECT_EQ(event.room_id, "!jEsUZKDJdhlrceRyVU:example.org") << "room id";
    EXPECT_EQ(event.sender, "@example:example.org") << "sender";
    EXPECT_EQ(event.type, EventType::CallInvite) << "type";
    EXPECT_EQ(event.unsigned_data.age, 1234) << "age";
    EXPECT_EQ(event.content.call_id, "c1591052749788") << "call id";
    EXPECT_EQ(event.content.offer.sdp, "v=0\r\no=- 6584580628695956864 2 IN IP4 127.0.0.1[...]")
      << "sdp";
    EXPECT_EQ(event.content.offer.type, voip::RTCSessionDescriptionInit::Type::Offer)
      << "offer type";
    EXPECT_EQ(event.content.version, "0") << "version";
    EXPECT_EQ(event.content.lifetime, 120000) << "lifetime";

    EXPECT_EQ(data, json(event)) << "event id";
}

TEST(VoIPEventsV0, CallCandidates)
{
    nlohmann::json data = R"({
          "content": {
            "call_id": "c1591052749788",
            "candidates": [
              {
                "sdpMid": "audio",
                "sdpMLineIndex": 0,
                "candidate": "candidate:863018703 1 udp 2122260223 10.9.64.156 43670 typ host generation 0"
              }
            ],
            "version": 0
          },
          "event_id": "$143273582443PhrSn:example.org",
          "origin_server_ts": 1432735824653,
          "room_id": "!jEsUZKDJdhlrceRyVU:example.org",
          "sender": "@example:example.org",
          "type": "m.call.candidates",
          "unsigned": {
            "age": 1234
          }
        })"_json;

    RoomEvent<voip::CallCandidates> event = data.get<RoomEvent<voip::CallCandidates>>();

    EXPECT_EQ(event.event_id, "$143273582443PhrSn:example.org");
    EXPECT_EQ(event.origin_server_ts, 1432735824653L);
    EXPECT_EQ(event.room_id, "!jEsUZKDJdhlrceRyVU:example.org");
    EXPECT_EQ(event.sender, "@example:example.org");
    EXPECT_EQ(event.type, EventType::CallCandidates);
    EXPECT_EQ(event.unsigned_data.age, 1234);
    EXPECT_EQ(event.content.call_id, "c1591052749788");
    EXPECT_EQ(event.content.candidates[0].sdpMid, "audio");
    EXPECT_EQ(event.content.candidates[0].sdpMLineIndex, 0);
    EXPECT_EQ(event.content.candidates[0].candidate,
              "candidate:863018703 1 udp 2122260223 10.9.64.156 43670 typ host generation 0");
    EXPECT_EQ(event.content.version, "0");

    EXPECT_EQ(data, json(event));
}

TEST(VoIPEventsV0, CallAnswer)
{
    nlohmann::json data = R"({
          "content": {
            "call_id": "c1591052749788",
            "answer": {
              "sdp": "v=0\r\no=- 6584580628695956864 2 IN IP4 127.0.0.1[...]",
              "type": "answer"
            },
            "version": 0
          },
          "event_id": "$143273582443PhrSn:example.org",
          "origin_server_ts": 1432735824653,
          "room_id": "!jEsUZKDJdhlrceRyVU:example.org",
          "sender": "@example:example.org",
          "type": "m.call.answer",
          "unsigned": {
            "age": 1234
          }
        })"_json;

    RoomEvent<voip::CallAnswer> event = data.get<RoomEvent<voip::CallAnswer>>();

    EXPECT_EQ(event.event_id, "$143273582443PhrSn:example.org");
    EXPECT_EQ(event.origin_server_ts, 1432735824653L);
    EXPECT_EQ(event.room_id, "!jEsUZKDJdhlrceRyVU:example.org");
    EXPECT_EQ(event.sender, "@example:example.org");
    EXPECT_EQ(event.type, EventType::CallAnswer);
    EXPECT_EQ(event.unsigned_data.age, 1234);
    EXPECT_EQ(event.content.call_id, "c1591052749788");
    EXPECT_EQ(event.content.answer.sdp, "v=0\r\no=- 6584580628695956864 2 IN IP4 127.0.0.1[...]");
    EXPECT_EQ(event.content.answer.type, voip::RTCSessionDescriptionInit::Type::Answer);
    EXPECT_EQ(event.content.version, "0");

    EXPECT_EQ(data, json(event));
}

TEST(VoIPEventsV0, CallHangUp)
{
    nlohmann::json data = R"({
          "content": {
            "call_id": "c1591052749788",
            "version": 0,
            "reason": "ice_failed"
          },
          "event_id": "$143273582443PhrSn:example.org",
          "origin_server_ts": 1432735824653,
          "room_id": "!jEsUZKDJdhlrceRyVU:example.org",
          "sender": "@example:example.org",
          "type": "m.call.hangup",
          "unsigned": {
            "age": 1234
          }
        })"_json;

    RoomEvent<voip::CallHangUp> event = data.get<RoomEvent<voip::CallHangUp>>();

    EXPECT_EQ(event.event_id, "$143273582443PhrSn:example.org");
    EXPECT_EQ(event.origin_server_ts, 1432735824653L);
    EXPECT_EQ(event.room_id, "!jEsUZKDJdhlrceRyVU:example.org");
    EXPECT_EQ(event.sender, "@example:example.org");
    EXPECT_EQ(event.type, EventType::CallHangUp);
    EXPECT_EQ(event.unsigned_data.age, 1234);
    EXPECT_EQ(event.content.call_id, "c1591052749788");
    EXPECT_EQ(event.content.version, "0");
    EXPECT_EQ(event.content.reason, voip::CallHangUp::Reason::ICEFailed);

    EXPECT_EQ(data, json(event));
}

//! Tests for VoIP v1 support
TEST(VoIPEventsV1, CallInvite)
{
    nlohmann::json data = R"({
          "content": {
            "call_id": "c1591052749788",
            "party_id": "o9124712020312",
            "offer": {
              "sdp": "v=0\r\no=- 6584580628695956864 2 IN IP4 127.0.0.1[...]",
              "type": "offer"
            },
            "version": "1",
            "lifetime": 120000,
            "invitee": "c1591052749784"
          },
          "event_id": "$143273582443PhrSn:example.org",
          "origin_server_ts": 1432735824653,
          "room_id": "!jEsUZKDJdhlrceRyVU:example.org",
          "sender": "@example:example.org",
          "type": "m.call.invite",
          "unsigned": {
            "age": 1234
          }
        })"_json;

    RoomEvent<voip::CallInvite> event = data.get<RoomEvent<voip::CallInvite>>();

    EXPECT_EQ(event.event_id, "$143273582443PhrSn:example.org");
    EXPECT_EQ(event.origin_server_ts, 1432735824653L);
    EXPECT_EQ(event.room_id, "!jEsUZKDJdhlrceRyVU:example.org");
    EXPECT_EQ(event.sender, "@example:example.org");
    EXPECT_EQ(event.type, EventType::CallInvite);
    EXPECT_EQ(event.unsigned_data.age, 1234);
    EXPECT_EQ(event.content.call_id, "c1591052749788");
    EXPECT_EQ(event.content.party_id, "o9124712020312");
    EXPECT_EQ(event.content.offer.sdp, "v=0\r\no=- 6584580628695956864 2 IN IP4 127.0.0.1[...]");
    EXPECT_EQ(event.content.offer.type, voip::RTCSessionDescriptionInit::Type::Offer);
    EXPECT_EQ(event.content.version, "1");
    EXPECT_EQ(event.content.lifetime, 120000);
    EXPECT_EQ(event.content.invitee, "c1591052749784");

    EXPECT_EQ(data, json(event));
}

TEST(VoIPEventsV1, CallCandidates)
{
    nlohmann::json data = R"({
          "content": {
            "call_id": "c1591052749788",
            "party_id": "o9124712020312",
            "candidates": [
              {
                "sdpMid": "audio",
                "sdpMLineIndex": 0,
                "candidate": "candidate:863018703 1 udp 2122260223 10.9.64.156 43670 typ host generation 0"
              }
            ],
            "version": "1"
          },
          "event_id": "$143273582443PhrSn:example.org",
          "origin_server_ts": 1432735824653,
          "room_id": "!jEsUZKDJdhlrceRyVU:example.org",
          "sender": "@example:example.org",
          "type": "m.call.candidates",
          "unsigned": {
            "age": 1234
          }
        })"_json;

    RoomEvent<voip::CallCandidates> event = data.get<RoomEvent<voip::CallCandidates>>();

    EXPECT_EQ(event.event_id, "$143273582443PhrSn:example.org");
    EXPECT_EQ(event.origin_server_ts, 1432735824653L);
    EXPECT_EQ(event.room_id, "!jEsUZKDJdhlrceRyVU:example.org");
    EXPECT_EQ(event.sender, "@example:example.org");
    EXPECT_EQ(event.type, EventType::CallCandidates);
    EXPECT_EQ(event.unsigned_data.age, 1234);
    EXPECT_EQ(event.content.call_id, "c1591052749788");
    EXPECT_EQ(event.content.party_id, "o9124712020312");
    EXPECT_EQ(event.content.candidates[0].sdpMid, "audio");
    EXPECT_EQ(event.content.candidates[0].sdpMLineIndex, 0);
    EXPECT_EQ(event.content.candidates[0].candidate,
              "candidate:863018703 1 udp 2122260223 10.9.64.156 43670 typ host generation 0");
    EXPECT_EQ(event.content.version, "1");

    EXPECT_EQ(data, json(event));
}

TEST(VoIPEventsV1, CallAnswer)
{
    nlohmann::json data = R"({
          "content": {
            "call_id": "c1591052749788",
            "party_id": "o9124712020312",
            "answer": {
              "sdp": "v=0\r\no=- 6584580628695956864 2 IN IP4 127.0.0.1[...]",
              "type": "answer"
            },
            "version": "1"
          },
          "event_id": "$143273582443PhrSn:example.org",
          "origin_server_ts": 1432735824653,
          "room_id": "!jEsUZKDJdhlrceRyVU:example.org",
          "sender": "@example:example.org",
          "type": "m.call.answer",
          "unsigned": {
            "age": 1234
          }
        })"_json;

    RoomEvent<voip::CallAnswer> event = data.get<RoomEvent<voip::CallAnswer>>();

    EXPECT_EQ(event.event_id, "$143273582443PhrSn:example.org");
    EXPECT_EQ(event.origin_server_ts, 1432735824653L);
    EXPECT_EQ(event.room_id, "!jEsUZKDJdhlrceRyVU:example.org");
    EXPECT_EQ(event.sender, "@example:example.org");
    EXPECT_EQ(event.type, EventType::CallAnswer);
    EXPECT_EQ(event.unsigned_data.age, 1234);
    EXPECT_EQ(event.content.call_id, "c1591052749788");
    EXPECT_EQ(event.content.party_id, "o9124712020312");
    EXPECT_EQ(event.content.answer.sdp, "v=0\r\no=- 6584580628695956864 2 IN IP4 127.0.0.1[...]");
    EXPECT_EQ(event.content.answer.type, voip::RTCSessionDescriptionInit::Type::Answer);
    EXPECT_EQ(event.content.version, "1");

    EXPECT_EQ(data, json(event));
}

TEST(VoIPEventsV1, CallHangUp)
{
    nlohmann::json data = R"({
          "content": {
            "call_id": "c1591052749788",
            "party_id": "o9124712020312",
            "version": "1",
            "reason": "user_media_failed"
          },
          "event_id": "$143273582443PhrSn:example.org",
          "origin_server_ts": 1432735824653,
          "room_id": "!jEsUZKDJdhlrceRyVU:example.org",
          "sender": "@example:example.org",
          "type": "m.call.hangup",
          "unsigned": {
            "age": 1234
          }
        })"_json;

    RoomEvent<voip::CallHangUp> event = data.get<RoomEvent<voip::CallHangUp>>();

    EXPECT_EQ(event.event_id, "$143273582443PhrSn:example.org");
    EXPECT_EQ(event.origin_server_ts, 1432735824653L);
    EXPECT_EQ(event.room_id, "!jEsUZKDJdhlrceRyVU:example.org");
    EXPECT_EQ(event.sender, "@example:example.org");
    EXPECT_EQ(event.type, EventType::CallHangUp);
    EXPECT_EQ(event.unsigned_data.age, 1234);
    EXPECT_EQ(event.content.call_id, "c1591052749788");
    EXPECT_EQ(event.content.party_id, "o9124712020312");
    EXPECT_EQ(event.content.version, "1");
    EXPECT_EQ(event.content.reason, voip::CallHangUp::Reason::UserMediaFailed);

    EXPECT_EQ(data, json(event));
}

TEST(VoIPEventsV1, CallSelectAnswer)
{
    nlohmann::json data = R"({
          "content": {
            "call_id": "c1591052749788",
            "party_id": "o9124712020312",
            "version": "1",
            "selected_party_id": "c2305207209345"
          },
          "event_id": "$143273582443PhrSn:example.org",
          "origin_server_ts": 1432735824653,
          "room_id": "!jEsUZKDJdhlrceRyVU:example.org",
          "sender": "@example:example.org",
          "type": "m.call.select_answer",
          "unsigned": {
            "age": 1234
          }
        })"_json;

    RoomEvent<voip::CallSelectAnswer> event = data.get<RoomEvent<voip::CallSelectAnswer>>();

    EXPECT_EQ(event.event_id, "$143273582443PhrSn:example.org");
    EXPECT_EQ(event.origin_server_ts, 1432735824653L);
    EXPECT_EQ(event.room_id, "!jEsUZKDJdhlrceRyVU:example.org");
    EXPECT_EQ(event.sender, "@example:example.org");
    EXPECT_EQ(event.type, EventType::CallSelectAnswer);
    EXPECT_EQ(event.unsigned_data.age, 1234);
    EXPECT_EQ(event.content.call_id, "c1591052749788");
    EXPECT_EQ(event.content.party_id, "o9124712020312");
    EXPECT_EQ(event.content.version, "1");
    EXPECT_EQ(event.content.selected_party_id, "c2305207209345");

    EXPECT_EQ(data, json(event));
}

TEST(VoIPEventsV1, CallReject)
{
    nlohmann::json data = R"({
          "content": {
            "call_id": "c1591052749788",
            "party_id": "o9124712020312",
            "version": "1"
          },
          "event_id": "$143273582443PhrSn:example.org",
          "origin_server_ts": 1432735824653,
          "room_id": "!jEsUZKDJdhlrceRyVU:example.org",
          "sender": "@example:example.org",
          "type": "m.call.reject",
          "unsigned": {
            "age": 1234
          }
        })"_json;

    RoomEvent<voip::CallReject> event = data.get<RoomEvent<voip::CallReject>>();

    EXPECT_EQ(event.event_id, "$143273582443PhrSn:example.org");
    EXPECT_EQ(event.origin_server_ts, 1432735824653L);
    EXPECT_EQ(event.room_id, "!jEsUZKDJdhlrceRyVU:example.org");
    EXPECT_EQ(event.sender, "@example:example.org");
    EXPECT_EQ(event.type, EventType::CallReject);
    EXPECT_EQ(event.unsigned_data.age, 1234);
    EXPECT_EQ(event.content.call_id, "c1591052749788");
    EXPECT_EQ(event.content.party_id, "o9124712020312");
    EXPECT_EQ(event.content.version, "1");

    EXPECT_EQ(data, json(event));
}
TEST(VoIPEventsV1, CallNegotiate)
{
    nlohmann::json data = R"({
          "content": {
            "call_id": "c1591052749788",
            "party_id": "o9124712020312",
            "description": {
              "sdp": "v=0\r\no=- 6584580628695956864 2 IN IP4 127.0.0.1[...]",
              "type": "offer"
            },
            "lifetime": 120000
          },
          "event_id": "$143273582443PhrSn:example.org",
          "origin_server_ts": 1432735824653,
          "room_id": "!jEsUZKDJdhlrceRyVU:example.org",
          "sender": "@example:example.org",
          "type": "m.call.negotiate",
          "unsigned": {
            "age": 1234
          }
        })"_json;

    RoomEvent<voip::CallNegotiate> event = data.get<RoomEvent<voip::CallNegotiate>>();

    EXPECT_EQ(event.event_id, "$143273582443PhrSn:example.org");
    EXPECT_EQ(event.origin_server_ts, 1432735824653L);
    EXPECT_EQ(event.room_id, "!jEsUZKDJdhlrceRyVU:example.org");
    EXPECT_EQ(event.sender, "@example:example.org");
    EXPECT_EQ(event.type, EventType::CallNegotiate);
    EXPECT_EQ(event.unsigned_data.age, 1234);
    EXPECT_EQ(event.content.call_id, "c1591052749788");
    EXPECT_EQ(event.content.party_id, "o9124712020312");
    EXPECT_EQ(event.content.description.sdp,
              "v=0\r\no=- 6584580628695956864 2 IN IP4 127.0.0.1[...]");
    EXPECT_EQ(event.content.description.type, voip::RTCSessionDescriptionInit::Type::Offer);
    EXPECT_EQ(event.content.lifetime, 120000);

    EXPECT_EQ(data, json(event));
}