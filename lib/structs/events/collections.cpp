#include "mtx/events/collections.hpp"
#include "mtx/events_impl.hpp"
#include "mtx/log.hpp"

#include <nlohmann/json.hpp>

namespace mtx::events {
using namespace mtx::events::collections;

#define MTXCLIENT_INSTANTIATE_JSON_FUNCTIONS(EventType, Content)                                   \
        template void to_json<Content>(nlohmann::json &, const EventType<Content> &);              \
        template void from_json<Content>(const nlohmann::json &, EventType<Content> &);

MTXCLIENT_INSTANTIATE_JSON_FUNCTIONS(events::StateEvent, states::Aliases)
MTXCLIENT_INSTANTIATE_JSON_FUNCTIONS(events::StateEvent, states::Avatar)
MTXCLIENT_INSTANTIATE_JSON_FUNCTIONS(events::StateEvent, states::CanonicalAlias)
MTXCLIENT_INSTANTIATE_JSON_FUNCTIONS(events::StateEvent, states::Create)
MTXCLIENT_INSTANTIATE_JSON_FUNCTIONS(events::StateEvent, states::Encryption)
MTXCLIENT_INSTANTIATE_JSON_FUNCTIONS(events::StateEvent, states::GuestAccess)
MTXCLIENT_INSTANTIATE_JSON_FUNCTIONS(events::StateEvent, states::HistoryVisibility)
MTXCLIENT_INSTANTIATE_JSON_FUNCTIONS(events::StateEvent, states::JoinRules)
MTXCLIENT_INSTANTIATE_JSON_FUNCTIONS(events::StateEvent, states::Member)
MTXCLIENT_INSTANTIATE_JSON_FUNCTIONS(events::StateEvent, states::Name)
MTXCLIENT_INSTANTIATE_JSON_FUNCTIONS(events::StateEvent, states::PinnedEvents)
MTXCLIENT_INSTANTIATE_JSON_FUNCTIONS(events::StateEvent, states::PowerLevels)
MTXCLIENT_INSTANTIATE_JSON_FUNCTIONS(events::StateEvent, states::Tombstone)
MTXCLIENT_INSTANTIATE_JSON_FUNCTIONS(events::StateEvent, states::Topic)
MTXCLIENT_INSTANTIATE_JSON_FUNCTIONS(events::StateEvent, msgs::Redacted)
MTXCLIENT_INSTANTIATE_JSON_FUNCTIONS(events::EncryptedEvent, msgs::Encrypted)
MTXCLIENT_INSTANTIATE_JSON_FUNCTIONS(events::EncryptedEvent, msgs::OlmEncrypted)
MTXCLIENT_INSTANTIATE_JSON_FUNCTIONS(events::RoomEvent, msgs::StickerImage)
MTXCLIENT_INSTANTIATE_JSON_FUNCTIONS(events::RoomEvent, msgs::Reaction)
MTXCLIENT_INSTANTIATE_JSON_FUNCTIONS(events::RoomEvent, msgs::Redacted)
MTXCLIENT_INSTANTIATE_JSON_FUNCTIONS(events::RoomEvent, msgs::Audio)
MTXCLIENT_INSTANTIATE_JSON_FUNCTIONS(events::RoomEvent, msgs::Emote)
MTXCLIENT_INSTANTIATE_JSON_FUNCTIONS(events::RoomEvent, msgs::File)
MTXCLIENT_INSTANTIATE_JSON_FUNCTIONS(events::RoomEvent, msgs::Image)
MTXCLIENT_INSTANTIATE_JSON_FUNCTIONS(events::RoomEvent, msgs::Notice)
MTXCLIENT_INSTANTIATE_JSON_FUNCTIONS(events::RoomEvent, msgs::Text)
MTXCLIENT_INSTANTIATE_JSON_FUNCTIONS(events::RoomEvent, msgs::Video)
MTXCLIENT_INSTANTIATE_JSON_FUNCTIONS(events::RoomEvent, msgs::KeyVerificationRequest)
MTXCLIENT_INSTANTIATE_JSON_FUNCTIONS(events::RoomEvent, msgs::KeyVerificationStart)
MTXCLIENT_INSTANTIATE_JSON_FUNCTIONS(events::RoomEvent, msgs::KeyVerificationReady)
MTXCLIENT_INSTANTIATE_JSON_FUNCTIONS(events::RoomEvent, msgs::KeyVerificationDone)
MTXCLIENT_INSTANTIATE_JSON_FUNCTIONS(events::RoomEvent, msgs::KeyVerificationAccept)
MTXCLIENT_INSTANTIATE_JSON_FUNCTIONS(events::RoomEvent, msgs::KeyVerificationCancel)
MTXCLIENT_INSTANTIATE_JSON_FUNCTIONS(events::RoomEvent, msgs::KeyVerificationKey)
MTXCLIENT_INSTANTIATE_JSON_FUNCTIONS(events::RoomEvent, msgs::KeyVerificationMac)
MTXCLIENT_INSTANTIATE_JSON_FUNCTIONS(events::RoomEvent, msgs::CallInvite)
MTXCLIENT_INSTANTIATE_JSON_FUNCTIONS(events::RoomEvent, msgs::CallCandidates)
MTXCLIENT_INSTANTIATE_JSON_FUNCTIONS(events::RoomEvent, msgs::CallAnswer)
MTXCLIENT_INSTANTIATE_JSON_FUNCTIONS(events::RoomEvent, msgs::CallHangUp)

MTXCLIENT_INSTANTIATE_JSON_FUNCTIONS(events::StrippedEvent, states::Aliases)
MTXCLIENT_INSTANTIATE_JSON_FUNCTIONS(events::StrippedEvent, states::Avatar)
MTXCLIENT_INSTANTIATE_JSON_FUNCTIONS(events::StrippedEvent, states::CanonicalAlias)
MTXCLIENT_INSTANTIATE_JSON_FUNCTIONS(events::StrippedEvent, states::Create)
MTXCLIENT_INSTANTIATE_JSON_FUNCTIONS(events::StrippedEvent, states::Encryption)
MTXCLIENT_INSTANTIATE_JSON_FUNCTIONS(events::StrippedEvent, states::GuestAccess)
MTXCLIENT_INSTANTIATE_JSON_FUNCTIONS(events::StrippedEvent, states::HistoryVisibility)
MTXCLIENT_INSTANTIATE_JSON_FUNCTIONS(events::StrippedEvent, states::JoinRules)
MTXCLIENT_INSTANTIATE_JSON_FUNCTIONS(events::StrippedEvent, states::Member)
MTXCLIENT_INSTANTIATE_JSON_FUNCTIONS(events::StrippedEvent, states::Name)
MTXCLIENT_INSTANTIATE_JSON_FUNCTIONS(events::StrippedEvent, states::PinnedEvents)
MTXCLIENT_INSTANTIATE_JSON_FUNCTIONS(events::StrippedEvent, states::PowerLevels)
MTXCLIENT_INSTANTIATE_JSON_FUNCTIONS(events::StrippedEvent, states::Tombstone)
MTXCLIENT_INSTANTIATE_JSON_FUNCTIONS(events::StrippedEvent, states::Topic)

MTXCLIENT_INSTANTIATE_JSON_FUNCTIONS(events::DeviceEvent, msgs::Encrypted)
MTXCLIENT_INSTANTIATE_JSON_FUNCTIONS(events::DeviceEvent, msgs::OlmEncrypted)
MTXCLIENT_INSTANTIATE_JSON_FUNCTIONS(events::DeviceEvent, msgs::KeyVerificationRequest)
MTXCLIENT_INSTANTIATE_JSON_FUNCTIONS(events::DeviceEvent, msgs::KeyVerificationStart)
MTXCLIENT_INSTANTIATE_JSON_FUNCTIONS(events::DeviceEvent, msgs::KeyVerificationReady)
MTXCLIENT_INSTANTIATE_JSON_FUNCTIONS(events::DeviceEvent, msgs::KeyVerificationDone)
MTXCLIENT_INSTANTIATE_JSON_FUNCTIONS(events::DeviceEvent, msgs::KeyVerificationAccept)
MTXCLIENT_INSTANTIATE_JSON_FUNCTIONS(events::DeviceEvent, msgs::KeyVerificationCancel)
MTXCLIENT_INSTANTIATE_JSON_FUNCTIONS(events::DeviceEvent, msgs::KeyVerificationKey)
MTXCLIENT_INSTANTIATE_JSON_FUNCTIONS(events::DeviceEvent, msgs::KeyVerificationMac)
MTXCLIENT_INSTANTIATE_JSON_FUNCTIONS(events::DeviceEvent, msgs::RoomKey)
MTXCLIENT_INSTANTIATE_JSON_FUNCTIONS(events::DeviceEvent, msgs::ForwardedRoomKey)
MTXCLIENT_INSTANTIATE_JSON_FUNCTIONS(events::DeviceEvent, msgs::KeyRequest)
MTXCLIENT_INSTANTIATE_JSON_FUNCTIONS(events::DeviceEvent, msgs::SecretRequest)
MTXCLIENT_INSTANTIATE_JSON_FUNCTIONS(events::DeviceEvent, msgs::SecretSend)

MTXCLIENT_INSTANTIATE_JSON_FUNCTIONS(events::EphemeralEvent, ephemeral::Typing)
MTXCLIENT_INSTANTIATE_JSON_FUNCTIONS(events::EphemeralEvent, ephemeral::Receipt)

MTXCLIENT_INSTANTIATE_JSON_FUNCTIONS(events::AccountDataEvent, mtx::events::account_data::Tags)
MTXCLIENT_INSTANTIATE_JSON_FUNCTIONS(events::AccountDataEvent, account_data::FullyRead)
MTXCLIENT_INSTANTIATE_JSON_FUNCTIONS(events::AccountDataEvent, pushrules::GlobalRuleset)
MTXCLIENT_INSTANTIATE_JSON_FUNCTIONS(events::AccountDataEvent,
                                     mtx::events::account_data::nheko_extensions::HiddenEvents)
MTXCLIENT_INSTANTIATE_JSON_FUNCTIONS(events::Event, presence::Presence)

MTXCLIENT_INSTANTIATE_JSON_FUNCTIONS(events::RedactionEvent, msg::Redaction)
}

namespace mtx::events::collections {
void
from_json(const json &obj, TimelineEvent &e)
{
        const auto type = mtx::events::getEventType(obj);
        using namespace mtx::events::state;
        using namespace mtx::events::msg;

        if (!obj.contains("content") || obj["content"].empty()) {
                if (obj.contains("state_key"))
                        e.data = events::StateEvent<msg::Redacted>(obj);
                else
                        e.data = events::RoomEvent<msg::Redacted>(obj);
                return;
        }

        switch (type) {
        case events::EventType::Reaction: {
                e.data = events::RoomEvent<Reaction>(obj);
                break;
        }
        case events::EventType::RoomAliases: {
                e.data = events::StateEvent<Aliases>(obj);
                break;
        }
        case events::EventType::RoomAvatar: {
                e.data = events::StateEvent<Avatar>(obj);
                break;
        }
        case events::EventType::RoomCanonicalAlias: {
                e.data = events::StateEvent<CanonicalAlias>(obj);
                break;
        }
        case events::EventType::RoomCreate: {
                e.data = events::StateEvent<Create>(obj);
                break;
        }
        case events::EventType::RoomEncrypted: {
                e.data = events::EncryptedEvent<mtx::events::msg::Encrypted>(obj);
                break;
        }
        case events::EventType::RoomEncryption: {
                e.data = events::StateEvent<Encryption>(obj);
                break;
        }
        case events::EventType::RoomGuestAccess: {
                e.data = events::StateEvent<GuestAccess>(obj);
                break;
        }
        case events::EventType::RoomHistoryVisibility: {
                e.data = events::StateEvent<HistoryVisibility>(obj);
                break;
        }
        case events::EventType::RoomJoinRules: {
                e.data = events::StateEvent<JoinRules>(obj);
                break;
        }
        case events::EventType::RoomMember: {
                e.data = events::StateEvent<Member>(obj);
                break;
        }
        case events::EventType::RoomName: {
                e.data = events::StateEvent<Name>(obj);
                break;
        }
        case events::EventType::RoomPowerLevels: {
                e.data = events::StateEvent<PowerLevels>(obj);
                break;
        }
        case events::EventType::RoomRedaction: {
                e.data = events::RedactionEvent<mtx::events::msg::Redaction>(obj);
                break;
        }
        case events::EventType::RoomTombstone: {
                e.data = events::StateEvent<Tombstone>(obj);
                break;
        }
        case events::EventType::RoomTopic: {
                e.data = events::StateEvent<Topic>(obj);
                break;
        }
        case events::EventType::KeyVerificationCancel: {
                e.data = events::RoomEvent<events::msg::KeyVerificationCancel>(obj);
                break;
        }
        case events::EventType::KeyVerificationRequest: {
                e.data = events::RoomEvent<events::msg::KeyVerificationRequest>(obj);
                break;
        }
        case events::EventType::KeyVerificationReady: {
                e.data = events::RoomEvent<events::msg::KeyVerificationReady>(obj);
                break;
        }
        case events::EventType::KeyVerificationStart: {
                e.data = events::RoomEvent<events::msg::KeyVerificationStart>(obj);
                break;
        }
        case events::EventType::KeyVerificationDone: {
                e.data = events::RoomEvent<events::msg::KeyVerificationDone>(obj);
                break;
        }
        case events::EventType::KeyVerificationKey: {
                e.data = events::RoomEvent<events::msg::KeyVerificationKey>(obj);
                break;
        }
        case events::EventType::KeyVerificationMac: {
                e.data = events::RoomEvent<events::msg::KeyVerificationMac>(obj);
                break;
        }
        case events::EventType::KeyVerificationAccept: {
                e.data = events::RoomEvent<events::msg::KeyVerificationAccept>(obj);
                break;
        }
        case events::EventType::RoomMessage: {
                using MsgType       = mtx::events::MessageType;
                const auto msg_type = mtx::events::getMessageType(obj.at("content"));

                if (msg_type == events::MessageType::Unknown) {
                        try {
                                auto unsigned_data =
                                  obj.at("unsigned").at("redacted_by").get<std::string>();

                                if (unsigned_data.empty())
                                        return;

                                e.data = events::RoomEvent<events::msg::Redacted>(obj);
                                return;
                        } catch (json::exception &err) {
                                mtx::utils::log::log_error(std::string("Invalid event type: ") +
                                                           err.what() + " " + obj.dump(2));
                                return;
                        }
                }

                switch (msg_type) {
                case MsgType::Audio: {
                        e.data = events::RoomEvent<events::msg::Audio>(obj);
                        break;
                }
                case MsgType::Emote: {
                        e.data = events::RoomEvent<events::msg::Emote>(obj);
                        break;
                }
                case MsgType::File: {
                        e.data = events::RoomEvent<events::msg::File>(obj);
                        break;
                }
                case MsgType::Image: {
                        e.data = events::RoomEvent<events::msg::Image>(obj);
                        break;
                }
                case MsgType::Location: {
                        /* events::RoomEvent<events::msg::Location> location = e; */
                        /* container.emplace_back(location); */
                        break;
                }
                case MsgType::Notice: {
                        e.data = events::RoomEvent<events::msg::Notice>(obj);
                        break;
                }
                case MsgType::Text: {
                        e.data = events::RoomEvent<events::msg::Text>(obj);
                        break;
                }
                case MsgType::Video: {
                        e.data = events::RoomEvent<events::msg::Video>(obj);
                        break;
                }
                case MsgType::KeyVerificationRequest: {
                        e.data = events::RoomEvent<events::msg::KeyVerificationRequest>(obj);
                        break;
                }
                case MsgType::Unknown:
                        return;
                }
                break;
        }
        case events::EventType::Sticker: {
                e.data = events::Sticker(obj);
                break;
        }
        case events::EventType::CallInvite: {
                e.data = events::RoomEvent<events::msg::CallInvite>(obj);
                break;
        }
        case events::EventType::CallCandidates: {
                e.data = events::RoomEvent<events::msg::CallCandidates>(obj);
                break;
        }
        case events::EventType::CallAnswer: {
                e.data = events::RoomEvent<events::msg::CallAnswer>(obj);
                break;
        }
        case events::EventType::CallHangUp: {
                e.data = events::RoomEvent<events::msg::CallHangUp>(obj);
                break;
        }
        case events::EventType::RoomPinnedEvents:
        case events::EventType::RoomKey:          // not part of the timeline
        case events::EventType::ForwardedRoomKey: // not part of the timeline
        case events::EventType::RoomKeyRequest:   // Not part of the timeline
        case events::EventType::Tag:              // Not part of the timeline
        case events::EventType::Presence:         // Not part of the timeline
        case events::EventType::PushRules:        // Not part of the timeline
        case events::EventType::SecretRequest:    // Not part of the timeline
        case events::EventType::SecretSend:       // Not part of the timeline
        case events::EventType::Typing:
        case events::EventType::Receipt:
        case events::EventType::FullyRead:
        case events::EventType::NhekoHiddenEvents:
        case events::EventType::Unsupported:
                return;
        }
}
}
