#include "mtx/events.hpp"

using json = nlohmann::json;

namespace mtx {
namespace events {

EventType
getEventType(const std::string &type)
{
        if (type == "m.room_key_request")
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
        else if (type == "m.room.redaction")
                return EventType::RoomRedaction;
        else if (type == "m.room.pinned_events")
                return EventType::RoomPinnedEvents;
        else if (type == "m.sticker")
                return EventType::Sticker;
        else if (type == "m.tag")
                return EventType::Tag;

        return EventType::Unsupported;
}

std::string
to_string(EventType type)
{
        switch (type) {
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
        case EventType::RoomRedaction:
                return "m.room.redaction";
        case EventType::RoomPinnedEvents:
                return "m.room.pinned_events";
        case EventType::Sticker:
                return "m.sticker";
        case EventType::Tag:
                return "m.tag";
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
        else if (type == "m.video")
                return MessageType::Video;

        return MessageType::Unknown;
}

MessageType
getMessageType(const json &obj)
{
        if (obj.is_null())
                return MessageType::Unknown;

        if (obj.find("msgtype") == obj.end())
                return MessageType::Unknown;

        return getMessageType(obj.at("msgtype").get<std::string>());
}
}
}
