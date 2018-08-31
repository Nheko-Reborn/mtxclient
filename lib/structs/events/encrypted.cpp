#include <string>

#include "mtx/events/encrypted.hpp"

static constexpr auto OLM_ALGO = "m.olm.v1.curve25519-aes-sha2";

namespace mtx {
namespace events {
namespace msg {

void
from_json(const json &obj, OlmCipherContent &msg)
{
        msg.body = obj.at("body").get<std::string>();
        msg.type = obj.at("type").get<uint8_t>();
}

void
to_json(json &obj, const OlmCipherContent &msg)
{
        obj["body"] = msg.body;
        obj["type"] = msg.type;
}

void
from_json(const json &obj, OlmEncrypted &msg)
{
        msg.algorithm  = OLM_ALGO;
        msg.sender_key = obj.at("sender_key").get<std::string>();
        msg.ciphertext =
          obj.at("ciphertext").get<std::map<OlmEncrypted::RecipientKey, OlmCipherContent>>();
}

void
to_json(json &obj, const OlmEncrypted &msg)
{
        obj["algorithm"]  = msg.algorithm;
        obj["sender_key"] = msg.sender_key;
        obj["ciphertext"] = msg.ciphertext;
}

void
from_json(const json &obj, Encrypted &content)
{
        content.algorithm  = obj.at("algorithm").get<std::string>();
        content.ciphertext = obj.at("ciphertext").get<std::string>();
        content.device_id  = obj.at("device_id").get<std::string>();
        content.sender_key = obj.at("sender_key").get<std::string>();
        content.session_id = obj.at("session_id").get<std::string>();
}

void
to_json(json &obj, const Encrypted &content)
{
        obj["algorithm"]  = content.algorithm;
        obj["ciphertext"] = content.ciphertext;
        obj["device_id"]  = content.device_id;
        obj["sender_key"] = content.sender_key;
        obj["session_id"] = content.session_id;
}

void
from_json(const json &obj, RoomKey &event)
{
        event.algorithm   = obj.at("algorithm").get<std::string>();
        event.room_id     = obj.at("room_id").get<std::string>();
        event.session_id  = obj.at("session_id").get<std::string>();
        event.session_key = obj.at("session_key").get<std::string>();
}

void
to_json(json &obj, const RoomKey &event)
{
        obj["algorithm"]   = event.algorithm;
        obj["room_id"]     = event.room_id;
        obj["session_id"]  = event.session_id;
        obj["session_key"] = event.session_key;
}

void
from_json(const json &obj, KeyRequest &event)
{
        event.sender = obj.at("sender");
        event.type   = mtx::events::getEventType(obj.at("type").get<std::string>());

        event.request_id           = obj.at("content").at("request_id");
        event.requesting_device_id = obj.at("content").at("requesting_device_id");

        auto action = obj.at("content").at("action").get<std::string>();
        if (action == "request") {
                event.action     = RequestAction::Request;
                event.room_id    = obj.at("content").at("body").at("room_id");
                event.sender_key = obj.at("content").at("body").at("sender_key");
                event.session_id = obj.at("content").at("body").at("session_id");
                event.algorithm  = obj.at("content").at("body").at("algorithm");
        } else if (action == "request_cancellation") {
                event.action = RequestAction::Cancellation;
        }
}

void
to_json(json &obj, const KeyRequest &event)
{
        obj = json::object();

        obj["sender"] = event.sender;
        obj["type"]   = to_string(event.type);

        obj["content"] = json::object();

        obj["content"]["request_id"]           = event.request_id;
        obj["content"]["requesting_device_id"] = event.requesting_device_id;

        switch (event.action) {
        case RequestAction::Request: {
                obj["content"]["body"] = json::object();

                obj["content"]["body"]["room_id"]    = event.room_id;
                obj["content"]["body"]["sender_key"] = event.sender_key;
                obj["content"]["body"]["session_id"] = event.session_id;
                obj["content"]["body"]["algorithm"]  = "m.megolm.v1.aes-sha2";

                obj["content"]["action"] = "request";
                break;
        }
        case RequestAction::Cancellation: {
                obj["content"]["action"] = "request_cancellation";
                break;
        }
        default:
                break;
        }
}

} // namespace msg
} // namespace events
} // namespace mtx
