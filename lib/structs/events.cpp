#include "mtx/events.hpp"

#include <nlohmann/json.hpp>

using json = nlohmann::json;

namespace mtx {
namespace events {

EventType
getEventType(const std::string &type)
{
    if (type == "m.key.verification.request")
        return EventType::KeyVerificationRequest;
    else if (type == "m.key.verification.start")
        return EventType::KeyVerificationStart;
    else if (type == "m.key.verification.accept")
        return EventType::KeyVerificationAccept;
    else if (type == "m.key.verification.ready")
        return EventType::KeyVerificationReady;
    else if (type == "m.key.verification.done")
        return EventType::KeyVerificationDone;
    else if (type == "m.key.verification.key")
        return EventType::KeyVerificationKey;
    else if (type == "m.key.verification.mac")
        return EventType::KeyVerificationMac;
    else if (type == "m.key.verification.cancel")
        return EventType::KeyVerificationCancel;
    else if (type == "m.reaction")
        return EventType::Reaction;
    else if (type == "m.room_key")
        return EventType::RoomKey;
    else if (type == "m.forwarded_room_key")
        return EventType::ForwardedRoomKey;
    else if (type == "m.room_key_request")
        return EventType::RoomKeyRequest;
    else if (type == "m.room.aliases")
        return EventType::RoomAliases;
    else if (type == "m.room.avatar")
        return EventType::RoomAvatar;
    else if (type == "m.room.canonical_alias")
        return EventType::RoomCanonicalAlias;
    else if (type == "m.room.create")
        return EventType::RoomCreate;
    else if (type == "m.room.encrypted")
        return EventType::RoomEncrypted;
    else if (type == "m.dummy")
        return EventType::Dummy;
    else if (type == "m.room.encryption")
        return EventType::RoomEncryption;
    else if (type == "m.room.guest_access")
        return EventType::RoomGuestAccess;
    else if (type == "m.room.history_visibility")
        return EventType::RoomHistoryVisibility;
    else if (type == "m.room.join_rules")
        return EventType::RoomJoinRules;
    else if (type == "m.room.member")
        return EventType::RoomMember;
    else if (type == "m.room.message")
        return EventType::RoomMessage;
    else if (type == "m.room.name")
        return EventType::RoomName;
    else if (type == "m.room.power_levels")
        return EventType::RoomPowerLevels;
    else if (type == "m.room.topic")
        return EventType::RoomTopic;
    else if (type == "m.widget")
        return EventType::Widget;
    else if (type == "im.vector.modular.widgets")
        return EventType::VectorWidget;
    else if (type == "m.room.redaction")
        return EventType::RoomRedaction;
    else if (type == "m.room.pinned_events")
        return EventType::RoomPinnedEvents;
    else if (type == "m.room.tombstone")
        return EventType::RoomTombstone;
    else if (type == "m.sticker")
        return EventType::Sticker;

    else if (type == "m.policy.rule.user")
        return EventType::PolicyRuleUser;
    else if (type == "m.policy.rule.room")
        return EventType::PolicyRuleRoom;
    else if (type == "m.policy.rule.server")
        return EventType::PolicyRuleServer;

    else if (type == "m.space.child")
        return EventType::SpaceChild;
    else if (type == "m.space.parent")
        return EventType::SpaceParent;

    else if (type == "m.tag")
        return EventType::Tag;
    else if (type == "m.direct")
        return EventType::Direct;
    else if (type == "m.presence")
        return EventType::Presence;
    else if (type == "m.push_rules")
        return EventType::PushRules;
    else if (type == "m.call.invite")
        return EventType::CallInvite;
    else if (type == "m.call.candidates")
        return EventType::CallCandidates;
    else if (type == "m.call.answer")
        return EventType::CallAnswer;
    else if (type == "m.call.hangup")
        return EventType::CallHangUp;
    else if (type == "m.call.select_answer")
        return EventType::CallSelectAnswer;
    else if (type == "m.call.reject")
        return EventType::CallReject;
    else if (type == "m.call.negotiate")
        return EventType::CallNegotiate;
    else if (type == "m.secret.request")
        return EventType::SecretRequest;
    else if (type == "m.secret.send")
        return EventType::SecretSend;
    else if (type == "m.typing")
        return EventType::Typing;
    else if (type == "m.receipt")
        return EventType::Receipt;
    else if (type == "m.fully_read")
        return EventType::FullyRead;
    else if (type == "im.nheko.hidden_events")
        return EventType::NhekoHiddenEvents;
    else if (type == "im.ponies.room_emotes")
        return EventType::ImagePackInRoom;
    else if (type == "im.ponies.user_emotes")
        return EventType::ImagePackInAccountData;
    else if (type == "im.ponies.emote_rooms")
        return EventType::ImagePackRooms;
    else
        return EventType::Unsupported;
}

std::string
to_string(EventType type)
{
    switch (type) {
    case EventType::KeyVerificationCancel:
        return "m.key.verification.cancel";
    case EventType::KeyVerificationRequest:
        return "m.key.verification.request";
    case EventType::KeyVerificationStart:
        return "m.key.verification.start";
    case EventType::KeyVerificationAccept:
        return "m.key.verification.accept";
    case EventType::KeyVerificationKey:
        return "m.key.verification.key";
    case EventType::KeyVerificationReady:
        return "m.key.verification.ready";
    case EventType::KeyVerificationDone:
        return "m.key.verification.done";
    case EventType::KeyVerificationMac:
        return "m.key.verification.mac";
    case EventType::Reaction:
        return "m.reaction";
    case EventType::RoomKey:
        return "m.room_key";
    case EventType::ForwardedRoomKey:
        return "m.forwarded_room_key";
    case EventType::RoomKeyRequest:
        return "m.room_key_request";
    case EventType::RoomAliases:
        return "m.room.aliases";
    case EventType::RoomAvatar:
        return "m.room.avatar";
    case EventType::RoomCanonicalAlias:
        return "m.room.canonical_alias";
    case EventType::RoomCreate:
        return "m.room.create";
    case EventType::RoomEncrypted:
        return "m.room.encrypted";
    case EventType::Dummy:
        return "m.dummy";
    case EventType::RoomEncryption:
        return "m.room.encryption";
    case EventType::RoomGuestAccess:
        return "m.room.guest_access";
    case EventType::RoomHistoryVisibility:
        return "m.room.history_visibility";
    case EventType::RoomJoinRules:
        return "m.room.join_rules";
    case EventType::RoomMember:
        return "m.room.member";
    case EventType::RoomMessage:
        return "m.room.message";
    case EventType::RoomName:
        return "m.room.name";
    case EventType::RoomPowerLevels:
        return "m.room.power_levels";
    case EventType::RoomTopic:
        return "m.room.topic";
    case EventType::Widget:
        return "m.widget";
    case EventType::VectorWidget:
        return "im.vector.modular.widgets";
    case EventType::RoomRedaction:
        return "m.room.redaction";
    case EventType::RoomPinnedEvents:
        return "m.room.pinned_events";
    case EventType::RoomTombstone:
        return "m.room.tombstone";
    case EventType::Sticker:
        return "m.sticker";

    case EventType::PolicyRuleUser:
        return "m.policy.rule.user";
    case EventType::PolicyRuleRoom:
        return "m.policy.rule.room";
    case EventType::PolicyRuleServer:
        return "m.policy.rule.server";

    case EventType::SpaceChild:
        return "m.space.child";
    case EventType::SpaceParent:
        return "m.space.parent";

    case EventType::Tag:
        return "m.tag";
    case EventType::Direct:
        return "m.direct";
    case EventType::Presence:
        return "m.presence";
    case EventType::PushRules:
        return "m.push_rules";
    case EventType::CallInvite:
        return "m.call.invite";
    case EventType::CallCandidates:
        return "m.call.candidates";
    case EventType::CallAnswer:
        return "m.call.answer";
    case EventType::CallHangUp:
        return "m.call.hangup";
    case EventType::CallSelectAnswer:
        return "m.call.select_answer";
    case EventType::CallReject:
        return "m.call.reject";
    case EventType::CallNegotiate:
        return "m.call.negotiate";
    case EventType::SecretRequest:
        return "m.secret.request";
    case EventType::SecretSend:
        return "m.secret.send";
    case EventType::Typing:
        return "m.typing";
    case EventType::Receipt:
        return "m.receipt";
    case EventType::FullyRead:
        return "m.fully_read";
    case EventType::NhekoHiddenEvents:
        return "im.nheko.hidden_events";
    case EventType::ImagePackInRoom:
        return "im.ponies.room_emotes";
    case EventType::ImagePackInAccountData:
        return "im.ponies.user_emotes";
    case EventType::ImagePackRooms:
        return "im.ponies.emote_rooms";
    case EventType::Unsupported:
        return "";
    }

    return "";
}

EventType
getEventType(const json &obj)
{
    if (obj.find("type") != obj.end())
        return getEventType(obj.at("type").get<std::string>());

    return EventType::Unsupported;
}

MessageType
getMessageType(const std::string &type)
{
    if (type == "m.audio")
        return MessageType::Audio;
    else if (type == "m.emote")
        return MessageType::Emote;
    else if (type == "m.file")
        return MessageType::File;
    else if (type == "m.image")
        return MessageType::Image;
    else if (type == "m.location")
        return MessageType::Location;
    else if (type == "m.notice")
        return MessageType::Notice;
    else if (type == "m.text")
        return MessageType::Text;
    else if (type == "nic.custom.confetti")
        return MessageType::Confetti;
    else if (type == "m.video")
        return MessageType::Video;
    else if (type == "m.key.verification.request")
        return MessageType::KeyVerificationRequest;

    return MessageType::Unknown;
}

MessageType
getMessageType(const json &obj)
{
    if (obj.is_null())
        return MessageType::Unknown;

    if (obj.find("msgtype") == obj.end())
        return MessageType::Unknown;

    try {
        return getMessageType(obj.at("msgtype").get<std::string>());
    } catch (...) {
        return MessageType::Unknown;
    }
}
}
}
