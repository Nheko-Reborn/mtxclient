#include <cstdint>
#include <string>

#include <nlohmann/json.hpp>

#include "mtx/events/encrypted.hpp"

static constexpr auto OLM_ALGO = "m.olm.v1.curve25519-aes-sha2";

namespace mtx {
namespace events {
namespace msg {

void
to_json(nlohmann::json &obj, const SASMethods &method)
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
from_json(const nlohmann::json &obj, SASMethods &method)
{
    if (obj.get<std::string>() == "decimal")
        method = SASMethods::Decimal;
    else if (obj.get<std::string>() == "emoji")
        method = SASMethods::Emoji;
    else
        method = SASMethods::Unsupported;
}

void
to_json(nlohmann::json &obj, const VerificationMethods &method)
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
from_json(const nlohmann::json &obj, VerificationMethods &method)
{
    if (obj.get<std::string>() == "m.sas.v1")
        method = VerificationMethods::SASv1;
    else
        method = VerificationMethods::Unsupported;
}

void
from_json(const nlohmann::json &obj, OlmCipherContent &msg)
{
    msg.body = obj.at("body").get<std::string>();
    msg.type = obj.at("type").get<uint8_t>();
}

void
to_json(nlohmann::json &obj, const OlmCipherContent &msg)
{
    obj["body"] = msg.body;
    obj["type"] = msg.type;
}

void
from_json(const nlohmann::json &obj, OlmEncrypted &msg)
{
    msg.algorithm  = OLM_ALGO;
    msg.sender_key = obj.at("sender_key").get<std::string>();
    msg.ciphertext =
      obj.at("ciphertext").get<std::map<OlmEncrypted::RecipientKey, OlmCipherContent>>();
}

void
to_json(nlohmann::json &obj, const OlmEncrypted &msg)
{
    obj["algorithm"]  = msg.algorithm;
    obj["sender_key"] = msg.sender_key;
    obj["ciphertext"] = msg.ciphertext;
}

void
from_json(const nlohmann::json &obj, Encrypted &content)
{
    content.algorithm  = obj.at("algorithm").get<std::string>();
    content.ciphertext = obj.at("ciphertext").get<std::string>();
    content.session_id = obj.at("session_id").get<std::string>();

    // MSC3700
    content.device_id  = obj.value("device_id", "");
    content.sender_key = obj.value("sender_key", "");

    content.relations = common::parse_relations(obj);
}

void
to_json(nlohmann::json &obj, const Encrypted &content)
{
    obj["algorithm"]  = content.algorithm;
    obj["ciphertext"] = content.ciphertext;

    // MSC3700
    if (!content.device_id.empty())
        obj["device_id"] = content.device_id;
    if (!content.sender_key.empty())
        obj["sender_key"] = content.sender_key;

    obj["session_id"] = content.session_id;

    // For encrypted events, only add releations, don't generate new_content and friends
    common::add_relations(obj, content.relations);
}

void
from_json(const nlohmann::json &, Dummy &)
{
}

void
to_json(nlohmann::json &obj, const Dummy &)
{
    obj = nlohmann::json::object();
}

void
from_json(const nlohmann::json &obj, RoomKey &event)
{
    event.algorithm   = obj.at("algorithm").get<std::string>();
    event.room_id     = obj.at("room_id").get<std::string>();
    event.session_id  = obj.at("session_id").get<std::string>();
    event.session_key = obj.at("session_key").get<std::string>();
}

void
to_json(nlohmann::json &obj, const RoomKey &event)
{
    obj["algorithm"]   = event.algorithm;
    obj["room_id"]     = event.room_id;
    obj["session_id"]  = event.session_id;
    obj["session_key"] = event.session_key;
}

void
from_json(const nlohmann::json &obj, ForwardedRoomKey &event)
{
    event.algorithm                  = obj.at("algorithm").get<std::string>();
    event.room_id                    = obj.at("room_id").get<std::string>();
    event.session_id                 = obj.at("session_id").get<std::string>();
    event.session_key                = obj.at("session_key").get<std::string>();
    event.sender_key                 = obj.at("sender_key").get<std::string>();
    event.sender_claimed_ed25519_key = obj.at("sender_claimed_ed25519_key").get<std::string>();
    event.forwarding_curve25519_key_chain =
      obj.at("forwarding_curve25519_key_chain").get<std::vector<std::string>>();
}

void
to_json(nlohmann::json &obj, const ForwardedRoomKey &event)
{
    obj["algorithm"]                       = event.algorithm;
    obj["room_id"]                         = event.room_id;
    obj["session_id"]                      = event.session_id;
    obj["session_key"]                     = event.session_key;
    obj["sender_key"]                      = event.sender_key;
    obj["sender_claimed_ed25519_key"]      = event.sender_claimed_ed25519_key;
    obj["forwarding_curve25519_key_chain"] = event.forwarding_curve25519_key_chain;
}

void
from_json(const nlohmann::json &obj, KeyRequest &event)
{
    event.request_id           = obj.at("request_id").get<std::string>();
    event.requesting_device_id = obj.at("requesting_device_id").get<std::string>();

    auto action = obj.at("action").get<std::string>();
    if (action == "request") {
        event.action     = RequestAction::Request;
        event.room_id    = obj.at("body").at("room_id").get<std::string>();
        event.sender_key = obj.at("body").value("sender_key", "");
        event.session_id = obj.at("body").at("session_id").get<std::string>();
        event.algorithm  = obj.at("body").at("algorithm").get<std::string>();
    } else if (action == "request_cancellation") {
        event.action = RequestAction::Cancellation;
    }
}

void
to_json(nlohmann::json &obj, const KeyRequest &event)
{
    obj = nlohmann::json::object();

    obj = nlohmann::json::object();

    obj["request_id"]           = event.request_id;
    obj["requesting_device_id"] = event.requesting_device_id;

    switch (event.action) {
    case RequestAction::Request: {
        obj["body"] = nlohmann::json::object();

        obj["body"]["room_id"] = event.room_id;

        // MSC3070
        if (!event.sender_key.empty())
            obj["body"]["sender_key"] = event.sender_key;

        obj["body"]["session_id"] = event.session_id;
        obj["body"]["algorithm"]  = "m.megolm.v1.aes-sha2";

        obj["action"] = "request";
        break;
    }
    case RequestAction::Cancellation: {
        obj["action"] = "request_cancellation";
        break;
    }
    default:
        break;
    }
}

void
from_json(const nlohmann::json &obj, KeyVerificationRequest &event)
{
    if (obj.count("body") != 0) {
        event.body = obj.at("body").get<std::string>();
    }
    event.from_device = obj.at("from_device").get<std::string>();
    event.methods     = obj.at("methods").get<std::vector<VerificationMethods>>();
    if (obj.count("timestamp") != 0) {
        event.timestamp = obj.at("timestamp").get<uint64_t>();
    }
    if (obj.count("msgtype") != 0) {
        event.msgtype = obj.at("msgtype").get<std::string>();
    }
    if (obj.count("to") != 0) {
        event.to = obj.at("to").get<std::string>();
    }
    if (obj.count("transaction_id") != 0) {
        event.transaction_id = obj.at("transaction_id").get<std::string>();
    }
}

void
to_json(nlohmann::json &obj, const KeyVerificationRequest &event)
{
    if (event.body.has_value())
        obj["body"] = event.body.value();
    obj["from_device"] = event.from_device;
    obj["methods"]     = event.methods;
    if (event.msgtype.has_value())
        obj["msgtype"] = "m.key.verification.request";
    if (event.timestamp.has_value())
        obj["timestamp"] = event.timestamp.value();
    if (event.to.has_value())
        obj["to"] = event.to.value();
    if (event.transaction_id.has_value())
        obj["transaction_id"] = event.transaction_id.value();
}

void
from_json(const nlohmann::json &obj, KeyVerificationStart &event)
{
    event.from_device = obj.at("from_device").get<std::string>();
    if (obj.count("transaction_id") != 0) {
        event.transaction_id = obj.at("transaction_id").get<std::string>();
    }
    event.method = obj.at("method").get<VerificationMethods>();
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
    event.relations = common::parse_relations(obj);
}

void
to_json(nlohmann::json &obj, const KeyVerificationStart &event)
{
    obj["from_device"] = event.from_device;
    obj["method"]      = event.method;
    if (event.transaction_id.has_value())
        obj["transaction_id"] = event.transaction_id.value();
    if (event.next_method.has_value())
        obj["next_method"] = event.next_method.value();
    obj["key_agreement_protocols"]      = event.key_agreement_protocols;
    obj["hashes"]                       = event.hashes;
    obj["message_authentication_codes"] = event.message_authentication_codes;
    obj["short_authentication_string"]  = event.short_authentication_string;
    common::apply_relations(obj, event.relations);
}

void
from_json(const nlohmann::json &obj, KeyVerificationReady &event)
{
    if (obj.count("transaction_id") != 0) {
        event.transaction_id = obj.at("transaction_id").get<std::string>();
    }
    event.methods     = obj.at("methods").get<std::vector<VerificationMethods>>();
    event.from_device = obj.at("from_device").get<std::string>();
    event.relations   = common::parse_relations(obj);
}

void
to_json(nlohmann::json &obj, const KeyVerificationReady &event)
{
    obj["methods"] = event.methods;
    if (event.transaction_id.has_value())
        obj["transaction_id"] = event.transaction_id.value();
    obj["from_device"] = event.from_device;
    common::apply_relations(obj, event.relations);
}

void
from_json(const nlohmann::json &obj, KeyVerificationDone &event)
{
    if (obj.count("transaction_id") != 0) {
        event.transaction_id = obj.at("transaction_id").get<std::string>();
    }
    event.relations = common::parse_relations(obj);
}

void
to_json(nlohmann::json &obj, const KeyVerificationDone &event)
{
    if (event.transaction_id.has_value())
        obj["transaction_id"] = event.transaction_id.value();
    common::apply_relations(obj, event.relations);
}

void
from_json(const nlohmann::json &obj, KeyVerificationAccept &event)
{
    if (obj.count("transaction_id") != 0) {
        event.transaction_id = obj.at("transaction_id").get<std::string>();
    }
    event.key_agreement_protocol      = obj.at("key_agreement_protocol").get<std::string>();
    event.hash                        = obj.at("hash").get<std::string>();
    event.message_authentication_code = obj.at("message_authentication_code").get<std::string>();
    event.short_authentication_string =
      obj.at("short_authentication_string").get<std::vector<SASMethods>>();
    event.commitment = obj.at("commitment").get<std::string>();
    event.method     = obj.value("method", VerificationMethods::SASv1);
    event.relations  = common::parse_relations(obj);
}

void
to_json(nlohmann::json &obj, const KeyVerificationAccept &event)
{
    if (event.transaction_id.has_value())
        obj["transaction_id"] = event.transaction_id.value();
    obj["key_agreement_protocol"]      = event.key_agreement_protocol;
    obj["hash"]                        = event.hash;
    obj["message_authentication_code"] = event.message_authentication_code;
    obj["short_authentication_string"] = event.short_authentication_string;
    obj["commitment"]                  = event.commitment;
    obj["method"]                      = event.method;
    common::apply_relations(obj, event.relations);
}

void
from_json(const nlohmann::json &obj, KeyVerificationCancel &event)
{
    if (obj.count("transaction_id") != 0) {
        event.transaction_id = obj.at("transaction_id").get<std::string>();
    }
    event.reason    = obj.value("reason", "");
    event.code      = obj.value("code", "");
    event.relations = common::parse_relations(obj);
}

void
to_json(nlohmann::json &obj, const KeyVerificationCancel &event)
{
    if (event.transaction_id.has_value())
        obj["transaction_id"] = event.transaction_id.value();
    obj["reason"] = event.reason;
    obj["code"]   = event.code;
    common::apply_relations(obj, event.relations);
}

void
from_json(const nlohmann::json &obj, KeyVerificationKey &event)
{
    if (obj.count("transaction_id") != 0) {
        event.transaction_id = obj.at("transaction_id").get<std::string>();
    }
    event.key       = obj.at("key").get<std::string>();
    event.relations = common::parse_relations(obj);
}

void
to_json(nlohmann::json &obj, const KeyVerificationKey &event)
{
    if (event.transaction_id.has_value())
        obj["transaction_id"] = event.transaction_id.value();
    obj["key"] = event.key;
    common::apply_relations(obj, event.relations);
}

void
from_json(const nlohmann::json &obj, KeyVerificationMac &event)
{
    if (obj.count("transaction_id") != 0) {
        event.transaction_id = obj.at("transaction_id").get<std::string>();
    }
    event.mac       = obj.at("mac").get<std::map<std::string, std::string>>();
    event.keys      = obj.at("keys").get<std::string>();
    event.relations = common::parse_relations(obj);
}

void
to_json(nlohmann::json &obj, const KeyVerificationMac &event)
{
    if (event.transaction_id.has_value())
        obj["transaction_id"] = event.transaction_id.value();
    obj["mac"]  = event.mac;
    obj["keys"] = event.keys;
    common::apply_relations(obj, event.relations);
}

void
from_json(const nlohmann::json &obj, SecretRequest &event)
{
    event.action = RequestAction::Unknown;
    auto action  = obj.value("action", "");
    if (action == "request") {
        event.action = RequestAction::Request;
    } else if (action == "request_cancellation") {
        event.action = RequestAction::Cancellation;
    }

    event.name = obj.value("name", "");

    event.request_id           = obj.value("request_id", "");
    event.requesting_device_id = obj.value("requesting_device_id", "");
}

void
to_json(nlohmann::json &obj, const SecretRequest &event)
{
    switch (event.action) {
    case RequestAction::Request:
        obj["action"] = "request";
        break;
    case RequestAction::Cancellation:
        obj["action"] = "request_cancellation";
        break;
    default:
        throw std::invalid_argument("Unknown secret request action type");
    }

    if (!event.name.empty())
        obj["name"] = event.name;

    obj["request_id"]           = event.request_id;
    obj["requesting_device_id"] = event.requesting_device_id;
}

void
from_json(const nlohmann::json &obj, SecretSend &event)
{
    event.request_id = obj.value("request_id", "");
    event.secret     = obj.value("secret", "");
}

void
to_json(nlohmann::json &obj, const SecretSend &event)
{
    obj["request_id"] = event.request_id;
    obj["secret"]     = event.secret;
}
} // namespace msg
} // namespace events
} // namespace mtx
