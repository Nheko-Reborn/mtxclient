#include "mtx/events/collections.hpp"
#include "mtx/events_impl.hpp"
#include "mtx/log.hpp"

#include <nlohmann/json.hpp>

namespace mtx::events {
using namespace mtx::events::collections;

#define MTXCLIENT_INSTANTIATE_JSON_FUNCTIONS(EventType, Content)                                   \
    template void to_json<Content>(nlohmann::json &, const EventType<Content> &);                  \
    template void from_json<Content>(const nlohmann::json &, EventType<Content> &);

MTXCLIENT_INSTANTIATE_JSON_FUNCTIONS(events::StateEvent, mtx::events::state::Aliases)
MTXCLIENT_INSTANTIATE_JSON_FUNCTIONS(events::StateEvent, mtx::events::state::Avatar)
MTXCLIENT_INSTANTIATE_JSON_FUNCTIONS(events::StateEvent, mtx::events::state::CanonicalAlias)
MTXCLIENT_INSTANTIATE_JSON_FUNCTIONS(events::StateEvent, mtx::events::state::Create)
MTXCLIENT_INSTANTIATE_JSON_FUNCTIONS(events::StateEvent, mtx::events::state::Encryption)
MTXCLIENT_INSTANTIATE_JSON_FUNCTIONS(events::StateEvent, mtx::events::state::GuestAccess)
MTXCLIENT_INSTANTIATE_JSON_FUNCTIONS(events::StateEvent, mtx::events::state::HistoryVisibility)
MTXCLIENT_INSTANTIATE_JSON_FUNCTIONS(events::StateEvent, mtx::events::state::JoinRules)
MTXCLIENT_INSTANTIATE_JSON_FUNCTIONS(events::StateEvent, mtx::events::state::Member)
MTXCLIENT_INSTANTIATE_JSON_FUNCTIONS(events::StateEvent, mtx::events::state::Name)
MTXCLIENT_INSTANTIATE_JSON_FUNCTIONS(events::StateEvent, mtx::events::state::PinnedEvents)
MTXCLIENT_INSTANTIATE_JSON_FUNCTIONS(events::StateEvent, mtx::events::state::PowerLevels)
MTXCLIENT_INSTANTIATE_JSON_FUNCTIONS(events::StateEvent, mtx::events::state::Tombstone)
MTXCLIENT_INSTANTIATE_JSON_FUNCTIONS(events::StateEvent, mtx::events::state::ServerAcl)
MTXCLIENT_INSTANTIATE_JSON_FUNCTIONS(events::StateEvent, mtx::events::state::Topic)
MTXCLIENT_INSTANTIATE_JSON_FUNCTIONS(events::StateEvent, mtx::events::state::Widget)
MTXCLIENT_INSTANTIATE_JSON_FUNCTIONS(events::StateEvent, mtx::events::state::policy_rule::UserRule)
MTXCLIENT_INSTANTIATE_JSON_FUNCTIONS(events::StateEvent, mtx::events::state::policy_rule::RoomRule)
MTXCLIENT_INSTANTIATE_JSON_FUNCTIONS(events::StateEvent,
                                     mtx::events::state::policy_rule::ServerRule)
MTXCLIENT_INSTANTIATE_JSON_FUNCTIONS(events::StateEvent, mtx::events::state::space::Child)
MTXCLIENT_INSTANTIATE_JSON_FUNCTIONS(events::StateEvent, mtx::events::state::space::Parent)
MTXCLIENT_INSTANTIATE_JSON_FUNCTIONS(events::StateEvent, mtx::events::msg::Redacted)
MTXCLIENT_INSTANTIATE_JSON_FUNCTIONS(events::StateEvent, msc2545::ImagePack)
MTXCLIENT_INSTANTIATE_JSON_FUNCTIONS(events::StateEvent, Unknown)
MTXCLIENT_INSTANTIATE_JSON_FUNCTIONS(events::EncryptedEvent, mtx::events::msg::Encrypted)
MTXCLIENT_INSTANTIATE_JSON_FUNCTIONS(events::EncryptedEvent, mtx::events::msg::OlmEncrypted)
MTXCLIENT_INSTANTIATE_JSON_FUNCTIONS(events::RoomEvent, mtx::events::msg::StickerImage)
MTXCLIENT_INSTANTIATE_JSON_FUNCTIONS(events::RoomEvent, mtx::events::msg::Reaction)
MTXCLIENT_INSTANTIATE_JSON_FUNCTIONS(events::RoomEvent, mtx::events::msg::Redacted)
MTXCLIENT_INSTANTIATE_JSON_FUNCTIONS(events::RoomEvent, mtx::events::msg::Audio)
MTXCLIENT_INSTANTIATE_JSON_FUNCTIONS(events::RoomEvent, mtx::events::msg::ElementEffect)
MTXCLIENT_INSTANTIATE_JSON_FUNCTIONS(events::RoomEvent, mtx::events::msg::Emote)
MTXCLIENT_INSTANTIATE_JSON_FUNCTIONS(events::RoomEvent, mtx::events::msg::File)
MTXCLIENT_INSTANTIATE_JSON_FUNCTIONS(events::RoomEvent, mtx::events::msg::Image)
MTXCLIENT_INSTANTIATE_JSON_FUNCTIONS(events::RoomEvent, mtx::events::msg::Notice)
MTXCLIENT_INSTANTIATE_JSON_FUNCTIONS(events::RoomEvent, mtx::events::msg::Text)
MTXCLIENT_INSTANTIATE_JSON_FUNCTIONS(events::RoomEvent, mtx::events::msg::Unknown)
MTXCLIENT_INSTANTIATE_JSON_FUNCTIONS(events::RoomEvent, mtx::events::msg::Video)
MTXCLIENT_INSTANTIATE_JSON_FUNCTIONS(events::RoomEvent, mtx::events::msg::KeyVerificationRequest)
MTXCLIENT_INSTANTIATE_JSON_FUNCTIONS(events::RoomEvent, mtx::events::msg::KeyVerificationStart)
MTXCLIENT_INSTANTIATE_JSON_FUNCTIONS(events::RoomEvent, mtx::events::msg::KeyVerificationReady)
MTXCLIENT_INSTANTIATE_JSON_FUNCTIONS(events::RoomEvent, mtx::events::msg::KeyVerificationDone)
MTXCLIENT_INSTANTIATE_JSON_FUNCTIONS(events::RoomEvent, mtx::events::msg::KeyVerificationAccept)
MTXCLIENT_INSTANTIATE_JSON_FUNCTIONS(events::RoomEvent, mtx::events::msg::KeyVerificationCancel)
MTXCLIENT_INSTANTIATE_JSON_FUNCTIONS(events::RoomEvent, mtx::events::msg::KeyVerificationKey)
MTXCLIENT_INSTANTIATE_JSON_FUNCTIONS(events::RoomEvent, mtx::events::msg::KeyVerificationMac)
MTXCLIENT_INSTANTIATE_JSON_FUNCTIONS(events::RoomEvent, mtx::events::voip::CallInvite)
MTXCLIENT_INSTANTIATE_JSON_FUNCTIONS(events::RoomEvent, mtx::events::voip::CallCandidates)
MTXCLIENT_INSTANTIATE_JSON_FUNCTIONS(events::RoomEvent, mtx::events::voip::CallAnswer)
MTXCLIENT_INSTANTIATE_JSON_FUNCTIONS(events::RoomEvent, mtx::events::voip::CallHangUp)
MTXCLIENT_INSTANTIATE_JSON_FUNCTIONS(events::RoomEvent, mtx::events::voip::CallSelectAnswer)
MTXCLIENT_INSTANTIATE_JSON_FUNCTIONS(events::RoomEvent, mtx::events::voip::CallReject)
MTXCLIENT_INSTANTIATE_JSON_FUNCTIONS(events::RoomEvent, mtx::events::voip::CallNegotiate)
MTXCLIENT_INSTANTIATE_JSON_FUNCTIONS(events::RoomEvent, Unknown)

MTXCLIENT_INSTANTIATE_JSON_FUNCTIONS(events::StrippedEvent, mtx::events::state::Aliases)
MTXCLIENT_INSTANTIATE_JSON_FUNCTIONS(events::StrippedEvent, mtx::events::state::Avatar)
MTXCLIENT_INSTANTIATE_JSON_FUNCTIONS(events::StrippedEvent, mtx::events::state::CanonicalAlias)
MTXCLIENT_INSTANTIATE_JSON_FUNCTIONS(events::StrippedEvent, mtx::events::state::Create)
MTXCLIENT_INSTANTIATE_JSON_FUNCTIONS(events::StrippedEvent, mtx::events::state::Encryption)
MTXCLIENT_INSTANTIATE_JSON_FUNCTIONS(events::StrippedEvent, mtx::events::state::GuestAccess)
MTXCLIENT_INSTANTIATE_JSON_FUNCTIONS(events::StrippedEvent, mtx::events::state::HistoryVisibility)
MTXCLIENT_INSTANTIATE_JSON_FUNCTIONS(events::StrippedEvent, mtx::events::state::JoinRules)
MTXCLIENT_INSTANTIATE_JSON_FUNCTIONS(events::StrippedEvent, mtx::events::state::Member)
MTXCLIENT_INSTANTIATE_JSON_FUNCTIONS(events::StrippedEvent, mtx::events::state::Name)
MTXCLIENT_INSTANTIATE_JSON_FUNCTIONS(events::StrippedEvent, mtx::events::state::PinnedEvents)
MTXCLIENT_INSTANTIATE_JSON_FUNCTIONS(events::StrippedEvent, mtx::events::state::PowerLevels)
MTXCLIENT_INSTANTIATE_JSON_FUNCTIONS(events::StrippedEvent, mtx::events::state::Tombstone)
MTXCLIENT_INSTANTIATE_JSON_FUNCTIONS(events::StrippedEvent, mtx::events::state::ServerAcl)
MTXCLIENT_INSTANTIATE_JSON_FUNCTIONS(events::StrippedEvent, mtx::events::state::Topic)
MTXCLIENT_INSTANTIATE_JSON_FUNCTIONS(events::StrippedEvent, mtx::events::state::Widget)
MTXCLIENT_INSTANTIATE_JSON_FUNCTIONS(events::StrippedEvent,
                                     mtx::events::state::policy_rule::UserRule)
MTXCLIENT_INSTANTIATE_JSON_FUNCTIONS(events::StrippedEvent,
                                     mtx::events::state::policy_rule::RoomRule)
MTXCLIENT_INSTANTIATE_JSON_FUNCTIONS(events::StrippedEvent,
                                     mtx::events::state::policy_rule::ServerRule)
MTXCLIENT_INSTANTIATE_JSON_FUNCTIONS(events::StrippedEvent, mtx::events::state::space::Child)
MTXCLIENT_INSTANTIATE_JSON_FUNCTIONS(events::StrippedEvent, mtx::events::state::space::Parent)
MTXCLIENT_INSTANTIATE_JSON_FUNCTIONS(events::StrippedEvent, msg::Redacted)
MTXCLIENT_INSTANTIATE_JSON_FUNCTIONS(events::StrippedEvent, Unknown)

MTXCLIENT_INSTANTIATE_JSON_FUNCTIONS(events::DeviceEvent, mtx::events::msg::Encrypted)
MTXCLIENT_INSTANTIATE_JSON_FUNCTIONS(events::DeviceEvent, mtx::events::msg::OlmEncrypted)
MTXCLIENT_INSTANTIATE_JSON_FUNCTIONS(events::DeviceEvent, mtx::events::msg::KeyVerificationRequest)
MTXCLIENT_INSTANTIATE_JSON_FUNCTIONS(events::DeviceEvent, mtx::events::msg::KeyVerificationStart)
MTXCLIENT_INSTANTIATE_JSON_FUNCTIONS(events::DeviceEvent, mtx::events::msg::KeyVerificationReady)
MTXCLIENT_INSTANTIATE_JSON_FUNCTIONS(events::DeviceEvent, mtx::events::msg::KeyVerificationDone)
MTXCLIENT_INSTANTIATE_JSON_FUNCTIONS(events::DeviceEvent, mtx::events::msg::KeyVerificationAccept)
MTXCLIENT_INSTANTIATE_JSON_FUNCTIONS(events::DeviceEvent, mtx::events::msg::KeyVerificationCancel)
MTXCLIENT_INSTANTIATE_JSON_FUNCTIONS(events::DeviceEvent, mtx::events::msg::KeyVerificationKey)
MTXCLIENT_INSTANTIATE_JSON_FUNCTIONS(events::DeviceEvent, mtx::events::msg::KeyVerificationMac)
MTXCLIENT_INSTANTIATE_JSON_FUNCTIONS(events::DeviceEvent, mtx::events::msg::RoomKey)
MTXCLIENT_INSTANTIATE_JSON_FUNCTIONS(events::DeviceEvent, mtx::events::msg::ForwardedRoomKey)
MTXCLIENT_INSTANTIATE_JSON_FUNCTIONS(events::DeviceEvent, mtx::events::msg::KeyRequest)
MTXCLIENT_INSTANTIATE_JSON_FUNCTIONS(events::DeviceEvent, mtx::events::msg::SecretRequest)
MTXCLIENT_INSTANTIATE_JSON_FUNCTIONS(events::DeviceEvent, mtx::events::msg::SecretSend)
MTXCLIENT_INSTANTIATE_JSON_FUNCTIONS(events::DeviceEvent, mtx::events::msg::Dummy)
MTXCLIENT_INSTANTIATE_JSON_FUNCTIONS(events::DeviceEvent, Unknown)

MTXCLIENT_INSTANTIATE_JSON_FUNCTIONS(events::EphemeralEvent, ephemeral::Typing)
MTXCLIENT_INSTANTIATE_JSON_FUNCTIONS(events::EphemeralEvent, ephemeral::Receipt)
MTXCLIENT_INSTANTIATE_JSON_FUNCTIONS(events::EphemeralEvent, Unknown)

MTXCLIENT_INSTANTIATE_JSON_FUNCTIONS(events::AccountDataEvent, mtx::events::account_data::Direct)
MTXCLIENT_INSTANTIATE_JSON_FUNCTIONS(events::AccountDataEvent, mtx::events::account_data::Tags)
MTXCLIENT_INSTANTIATE_JSON_FUNCTIONS(events::AccountDataEvent, mtx::events::account_data::FullyRead)
MTXCLIENT_INSTANTIATE_JSON_FUNCTIONS(events::AccountDataEvent, pushrules::GlobalRuleset)
MTXCLIENT_INSTANTIATE_JSON_FUNCTIONS(events::AccountDataEvent,
                                     mtx::events::account_data::nheko_extensions::HiddenEvents)
MTXCLIENT_INSTANTIATE_JSON_FUNCTIONS(events::AccountDataEvent,
                                     mtx::events::account_data::nheko_extensions::EventExpiry)
MTXCLIENT_INSTANTIATE_JSON_FUNCTIONS(events::AccountDataEvent, msc2545::ImagePackRooms)
MTXCLIENT_INSTANTIATE_JSON_FUNCTIONS(events::AccountDataEvent, msc2545::ImagePack)
MTXCLIENT_INSTANTIATE_JSON_FUNCTIONS(events::Event, presence::Presence)

MTXCLIENT_INSTANTIATE_JSON_FUNCTIONS(events::RedactionEvent, msg::Redaction)
}

namespace mtx::events::collections {
void
to_json(nlohmann::json &obj, const TimelineEvents &e)
{
    std::visit([&obj](const auto &ev) { return to_json(obj, ev); }, e);
}

void
from_json(const nlohmann::json &obj, TimelineEvents &e)
{
    const auto type = mtx::events::getEventType(obj);
    using namespace mtx::events::state;
    using namespace mtx::events::msg;
    using namespace mtx::events::voip;

    try {
        if (obj.contains("unsigned") && obj["unsigned"].contains("redacted_by")) {
            if (obj.contains("state_key"))
                e = events::StateEvent<msg::Redacted>(obj);
            else
                e = events::RoomEvent<msg::Redacted>(obj);
            return;
        }

        switch (type) {
        case events::EventType::Reaction: {
            e = events::RoomEvent<Reaction>(obj);
            break;
        }
        case events::EventType::RoomAliases: {
            e = events::StateEvent<Aliases>(obj);
            break;
        }
        case events::EventType::RoomAvatar: {
            e = events::StateEvent<Avatar>(obj);
            break;
        }
        case events::EventType::RoomCanonicalAlias: {
            e = events::StateEvent<CanonicalAlias>(obj);
            break;
        }
        case events::EventType::RoomCreate: {
            e = events::StateEvent<Create>(obj);
            break;
        }
        case events::EventType::RoomEncrypted: {
            e = events::EncryptedEvent<mtx::events::msg::Encrypted>(obj);
            break;
        }
        case events::EventType::RoomEncryption: {
            e = events::StateEvent<Encryption>(obj);
            break;
        }
        case events::EventType::RoomGuestAccess: {
            e = events::StateEvent<GuestAccess>(obj);
            break;
        }
        case events::EventType::RoomHistoryVisibility: {
            e = events::StateEvent<HistoryVisibility>(obj);
            break;
        }
        case events::EventType::RoomJoinRules: {
            e = events::StateEvent<JoinRules>(obj);
            break;
        }
        case events::EventType::RoomMember: {
            e = events::StateEvent<Member>(obj);
            break;
        }
        case events::EventType::RoomName: {
            e = events::StateEvent<Name>(obj);
            break;
        }
        case events::EventType::RoomPowerLevels: {
            e = events::StateEvent<PowerLevels>(obj);
            break;
        }
        case events::EventType::RoomRedaction: {
            e = events::RedactionEvent<mtx::events::msg::Redaction>(obj);
            break;
        }
        case events::EventType::RoomTombstone: {
            e = events::StateEvent<Tombstone>(obj);
            break;
        }
        case events::EventType::RoomServerAcl: {
            e = events::StateEvent<ServerAcl>(obj);
            break;
        }
        case events::EventType::RoomTopic: {
            e = events::StateEvent<Topic>(obj);
            break;
        }
        case events::EventType::Widget: {
            e = events::StateEvent<Widget>(obj);
            break;
        }
        case events::EventType::VectorWidget: {
            e = events::StateEvent<Widget>(obj);
            break;
        }
        case events::EventType::RoomPinnedEvents: {
            e = events::StateEvent<PinnedEvents>(obj);
            break;
        }
        case events::EventType::PolicyRuleUser: {
            e = events::StateEvent<policy_rule::UserRule>(obj);
            break;
        }
        case events::EventType::PolicyRuleRoom: {
            e = events::StateEvent<policy_rule::RoomRule>(obj);
            break;
        }
        case events::EventType::PolicyRuleServer: {
            e = events::StateEvent<policy_rule::ServerRule>(obj);
            break;
        }
        case events::EventType::SpaceChild: {
            e = events::StateEvent<space::Child>(obj);
            break;
        }
        case events::EventType::SpaceParent: {
            e = events::StateEvent<space::Parent>(obj);
            break;
        }
        case events::EventType::ImagePackInRoom: {
            e = events::StateEvent<msc2545::ImagePack>(obj);
            break;
        }
        case events::EventType::KeyVerificationCancel: {
            e = events::RoomEvent<events::msg::KeyVerificationCancel>(obj);
            break;
        }
        case events::EventType::KeyVerificationRequest: {
            e = events::RoomEvent<events::msg::KeyVerificationRequest>(obj);
            break;
        }
        case events::EventType::KeyVerificationReady: {
            e = events::RoomEvent<events::msg::KeyVerificationReady>(obj);
            break;
        }
        case events::EventType::KeyVerificationStart: {
            e = events::RoomEvent<events::msg::KeyVerificationStart>(obj);
            break;
        }
        case events::EventType::KeyVerificationDone: {
            e = events::RoomEvent<events::msg::KeyVerificationDone>(obj);
            break;
        }
        case events::EventType::KeyVerificationKey: {
            e = events::RoomEvent<events::msg::KeyVerificationKey>(obj);
            break;
        }
        case events::EventType::KeyVerificationMac: {
            e = events::RoomEvent<events::msg::KeyVerificationMac>(obj);
            break;
        }
        case events::EventType::KeyVerificationAccept: {
            e = events::RoomEvent<events::msg::KeyVerificationAccept>(obj);
            break;
        }
        case events::EventType::RoomMessage: {
            using MsgType       = mtx::events::MessageType;
            const auto msg_type = mtx::events::getMessageType(obj.at("content"));

            switch (msg_type) {
            case MsgType::Audio: {
                e = events::RoomEvent<events::msg::Audio>(obj);
                break;
            }
            case MsgType::ElementEffect: {
                e = events::RoomEvent<events::msg::ElementEffect>(obj);
                break;
            }
            case MsgType::Emote: {
                e = events::RoomEvent<events::msg::Emote>(obj);
                break;
            }
            case MsgType::File: {
                e = events::RoomEvent<events::msg::File>(obj);
                break;
            }
            case MsgType::Image: {
                e = events::RoomEvent<events::msg::Image>(obj);
                break;
            }
            case MsgType::Location: {
                /* events::RoomEvent<events::msg::Location> location = e; */
                /* container.emplace_back(location); */
                break;
            }
            case MsgType::Notice: {
                e = events::RoomEvent<events::msg::Notice>(obj);
                break;
            }
            case MsgType::Text: {
                e = events::RoomEvent<events::msg::Text>(obj);
                break;
            }
            case MsgType::Video: {
                e = events::RoomEvent<events::msg::Video>(obj);
                break;
            }
            case MsgType::KeyVerificationRequest: {
                e = events::RoomEvent<events::msg::KeyVerificationRequest>(obj);
                break;
            }
            case MsgType::Unknown: {
                e = events::RoomEvent<events::msg::Unknown>(obj);
                break;
            }
            case MsgType::Redacted: {
                e = events::RoomEvent<events::Unknown>(obj);
                break;
            }
            case MsgType::Invalid:
                break;
            }
            break;
        }
        case events::EventType::Sticker: {
            e = events::Sticker(obj);
            break;
        }
        case events::EventType::CallInvite: {
            e = events::RoomEvent<events::voip::CallInvite>(obj);
            break;
        }
        case events::EventType::CallCandidates: {
            e = events::RoomEvent<events::voip::CallCandidates>(obj);
            break;
        }
        case events::EventType::CallAnswer: {
            e = events::RoomEvent<events::voip::CallAnswer>(obj);
            break;
        }
        case events::EventType::CallHangUp: {
            e = events::RoomEvent<events::voip::CallHangUp>(obj);
            break;
        }
        case events::EventType::CallSelectAnswer: {
            e = events::RoomEvent<events::voip::CallSelectAnswer>(obj);
            break;
        }
        case events::EventType::CallReject: {
            e = events::RoomEvent<events::voip::CallReject>(obj);
            break;
        }
        case events::EventType::CallNegotiate: {
            e = events::RoomEvent<events::voip::CallNegotiate>(obj);
            break;
        }
        case events::EventType::Unsupported: {
            e = events::RoomEvent<events::Unknown>(obj);
            break;
        }
        case events::EventType::RoomKey:          // not part of the timeline
        case events::EventType::ForwardedRoomKey: // not part of the timeline
        case events::EventType::RoomKeyRequest:   // Not part of the timeline
        case events::EventType::Direct:           // Not part of the timeline
        case events::EventType::Tag:              // Not part of the timeline
        case events::EventType::Presence:         // Not part of the timeline
        case events::EventType::PushRules:        // Not part of the timeline
        case events::EventType::SecretRequest:    // Not part of the timeline
        case events::EventType::SecretSend:       // Not part of the timeline
        case events::EventType::Typing:
        case events::EventType::Receipt:
        case events::EventType::FullyRead:
        case events::EventType::NhekoHiddenEvents:
        case events::EventType::NhekoEventExpiry:
        case events::EventType::ImagePackInAccountData:
        case events::EventType::ImagePackRooms:
        case events::EventType::Dummy:
            break;
        }
    } catch (std::exception &err) {
        mtx::utils::log::log()->error("Invalid event type: {} {}", err.what(), obj.dump(2));
        e = events::RoomEvent<events::Unknown>(obj);
    }
}
}
