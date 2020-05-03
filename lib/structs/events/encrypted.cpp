#include <string>

#include "mtx/events/encrypted.hpp"

static constexpr auto OLM_ALGO = "m.olm.v1.curve25519-aes-sha2";

namespace mtx {
namespace events {
namespace msg {

void
to_json(json &obj, const SASMethods &method)
{
        switch (method) {
        case SASMethods::Decimal:
                obj = "decimal";
                break;
        case SASMethods::Emoji:
                obj = "emoji";
                break;
        case SASMethods::Unsupported:
        default:
                obj = "unsupported";
                break;
        }
}

void
from_json(const json &obj, SASMethods &method)
{
        if (obj.get<std::string>() == "decimal")
                method = SASMethods::Decimal;
        else if (obj.get<std::string>() == "emoji")
                method = SASMethods::Emoji;
        else
                method = SASMethods::Unsupported;
}

void
to_json(json &obj, const VerificationMethods &method)
{
        switch (method) {
        case VerificationMethods::SASv1:
                obj = "m.sas.v1";
                break;
        case VerificationMethods::Unsupported:
        default:
                obj = "unsupported";
                break;
        }
}

void
from_json(const json &obj, VerificationMethods &method)
{
        if (obj.get<std::string>() == "m.sas.v1")
                method = VerificationMethods::SASv1;
        else
                method = VerificationMethods::Unsupported;
}

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

        if (obj.count("m.relates_to") != 0)
                content.relates_to = obj.at("m.relates_to").get<common::ReplyRelatesTo>();
}

void
to_json(json &obj, const Encrypted &content)
{
        obj["algorithm"]  = content.algorithm;
        obj["ciphertext"] = content.ciphertext;
        obj["device_id"]  = content.device_id;
        obj["sender_key"] = content.sender_key;
        obj["session_id"] = content.session_id;

        if (!content.relates_to.in_reply_to.event_id.empty())
                obj["m.relates_to"] = content.relates_to;
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
        event.sender               = obj.at("sender");
        event.type                 = mtx::events::getEventType(obj.at("type").get<std::string>());
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

        obj["sender"]  = event.sender;
        obj["type"]    = to_string(event.type);
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

void
from_json(const json &obj, KeyVerificationRequest &event)
{
        event.from_device    = obj.at("from_device").get<std::string>();
        event.methods        = obj.at("methods").get<std::vector<VerificationMethods>>();
        event.timestamp      = obj.at("timestamp").get<uint64_t>();
        event.transaction_id = obj.at("transaction_id").get<std::string>();
}

void
to_json(json &obj, const KeyVerificationRequest &event)
{
        obj["from_device"]    = event.from_device;
        obj["methods"]        = event.methods;
        obj["timestamp"]      = event.timestamp;
        obj["transaction_id"] = event.transaction_id;
}

void
from_json(const json &obj, KeyVerificationStart &event)
{
        event.from_device    = obj.at("from_device").get<std::string>();
        event.transaction_id = obj.at("transaction_id").get<std::string>();
        event.method         = obj.at("method").get<VerificationMethods>();
        if (obj.count("next_method") != 0) {
                event.next_method = obj.at("next_method").get<std::string>();
        }
        event.key_agreement_protocols =
          obj.at("key_agreement_protocols").get<std::vector<std::string>>();
        event.hashes = obj.at("hashes").get<std::vector<std::string>>();
        event.message_authentication_codes =
          obj.at("message_authentication_codes").get<std::vector<std::string>>();
        event.short_authentication_string =
          obj.at("short_authentication_string").get<std::vector<SASMethods>>();
}

void
to_json(json &obj, const KeyVerificationStart &event)
{
        obj["from_device"]    = event.from_device;
        obj["method"]         = event.method;
        obj["transaction_id"] = event.transaction_id;
        if (event.next_method.has_value())
                obj["next_method"] = event.next_method.value();
        obj["key_agreement_protocols"]      = event.key_agreement_protocols;
        obj["hashes"]                       = event.hashes;
        obj["message_authentication_codes"] = event.message_authentication_codes;
        obj["short_authentication_string"]  = event.short_authentication_string;
}

void
from_json(const json &obj, KeyVerificationAccept &event)
{
        event.transaction_id         = obj.at("transaction_id").get<std::string>();
        event.method                 = obj.at("method").get<VerificationMethods>();
        event.key_agreement_protocol = obj.at("key_agreement_protocol").get<std::string>();
        event.hash                   = obj.at("hash").get<std::string>();
        event.message_authentication_code =
          obj.at("message_authentication_code").get<std::string>();
        event.short_authentication_string =
          obj.at("short_authentication_string").get<std::vector<SASMethods>>();
        event.commitment = obj.at("commitment").get<std::string>();
}

void
to_json(json &obj, const KeyVerificationAccept &event)
{
        obj["method"]                      = event.method;
        obj["transaction_id"]              = event.transaction_id;
        obj["key_agreement_protocol"]      = event.key_agreement_protocol;
        obj["hash"]                        = event.hash;
        obj["message_authentication_code"] = event.message_authentication_code;
        obj["short_authentication_string"] = event.short_authentication_string;
        obj["commitment"]                  = event.commitment;
}

void
from_json(const json &obj, KeyVerificationCancel &event)
{
        event.transaction_id = obj.at("transaction_id").get<std::string>();
        event.reason         = obj.at("reason").get<std::string>();
        event.code           = obj.at("code").get<std::string>();
}

void
to_json(json &obj, const KeyVerificationCancel &event)
{
        obj["transaction_id"] = event.transaction_id;
        obj["reason"]         = event.reason;
        obj["code"]           = event.code;
}

void
from_json(const json &obj, KeyVerificationKey &event)
{
        event.transaction_id = obj.at("transaction_id").get<std::string>();
        event.key            = obj.at("key").get<std::string>();
}

void
to_json(json &obj, const KeyVerificationKey &event)
{
        obj["transaction_id"] = event.transaction_id;
        obj["key"]            = event.key;
}

void
from_json(const json &obj, KeyVerificationMac &event)
{
        event.transaction_id = obj.at("transaction_id").get<std::string>();
        event.mac            = obj.at("mac").get<std::map<std::string, std::string>>();
        event.keys           = obj.at("keys").get<std::string>();
}

void
to_json(json &obj, const KeyVerificationMac &event)
{
        obj["transaction_id"] = event.transaction_id;
        obj["mac"]            = event.mac;
        obj["keys"]           = event.keys;
}

} // namespace msg
} // namespace events
} // namespace mtx
