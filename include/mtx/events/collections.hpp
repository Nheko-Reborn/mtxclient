#pragma once

#include <variant>

#include "mtx/events.hpp"
#include "mtx/events/aliases.hpp"
#include "mtx/events/avatar.hpp"
#include "mtx/events/canonical_alias.hpp"
#include "mtx/events/create.hpp"
#include "mtx/events/encrypted.hpp"
#include "mtx/events/encryption.hpp"
#include "mtx/events/guest_access.hpp"
#include "mtx/events/history_visibility.hpp"
#include "mtx/events/join_rules.hpp"
#include "mtx/events/member.hpp"
#include "mtx/events/name.hpp"
#include "mtx/events/pinned_events.hpp"
#include "mtx/events/power_levels.hpp"
#include "mtx/events/redaction.hpp"
#include "mtx/events/tag.hpp"
#include "mtx/events/tombstone.hpp"
#include "mtx/events/topic.hpp"

#include "mtx/events/messages/audio.hpp"
#include "mtx/events/messages/emote.hpp"
#include "mtx/events/messages/file.hpp"
#include "mtx/events/messages/image.hpp"
#include "mtx/events/messages/notice.hpp"
#include "mtx/events/messages/text.hpp"
#include "mtx/events/messages/video.hpp"

namespace mtx {
namespace events {

//! Contains heterogeneous collections of events using std::variant.
namespace collections {

namespace account_data = mtx::events::account_data;
namespace states       = mtx::events::state;
namespace msgs         = mtx::events::msg;

//! Collection of room specific account data
using RoomAccountDataEvents = std::variant<events::Event<account_data::Tag>>;

//! Collection of @p StateEvent only.
using StateEvents = std::variant<events::StateEvent<states::Aliases>,
                                 events::StateEvent<states::Avatar>,
                                 events::StateEvent<states::CanonicalAlias>,
                                 events::StateEvent<states::Create>,
                                 events::StateEvent<states::Encryption>,
                                 events::StateEvent<states::GuestAccess>,
                                 events::StateEvent<states::HistoryVisibility>,
                                 events::StateEvent<states::JoinRules>,
                                 events::StateEvent<states::Member>,
                                 events::StateEvent<states::Name>,
                                 events::StateEvent<states::PinnedEvents>,
                                 events::StateEvent<states::PowerLevels>,
                                 events::StateEvent<states::Tombstone>,
                                 events::StateEvent<states::Topic>,
                                 events::StateEvent<msgs::Redacted>>;

//! Collection of @p StrippedEvent only.
using StrippedEvents = std::variant<events::StrippedEvent<states::Aliases>,
                                    events::StrippedEvent<states::Avatar>,
                                    events::StrippedEvent<states::CanonicalAlias>,
                                    events::StrippedEvent<states::Create>,
                                    events::StrippedEvent<states::GuestAccess>,
                                    events::StrippedEvent<states::HistoryVisibility>,
                                    events::StrippedEvent<states::JoinRules>,
                                    events::StrippedEvent<states::Member>,
                                    events::StrippedEvent<states::Name>,
                                    events::StrippedEvent<states::PinnedEvents>,
                                    events::StrippedEvent<states::PowerLevels>,
                                    events::StrippedEvent<states::Tombstone>,
                                    events::StrippedEvent<states::Topic>>;

//! Collection of @p StateEvent and @p RoomEvent. Those events would be
//! available on the returned timeline.
using TimelineEvents = std::variant<events::StateEvent<states::Aliases>,
                                    events::StateEvent<states::Avatar>,
                                    events::StateEvent<states::CanonicalAlias>,
                                    events::StateEvent<states::Create>,
                                    events::StateEvent<states::Encryption>,
                                    events::StateEvent<states::GuestAccess>,
                                    events::StateEvent<states::HistoryVisibility>,
                                    events::StateEvent<states::JoinRules>,
                                    events::StateEvent<states::Member>,
                                    events::StateEvent<states::Name>,
                                    events::StateEvent<states::PinnedEvents>,
                                    events::StateEvent<states::PowerLevels>,
                                    events::StateEvent<states::Tombstone>,
                                    events::StateEvent<states::Topic>,
                                    events::EncryptedEvent<msgs::Encrypted>,
                                    events::RedactionEvent<msgs::Redaction>,
                                    events::Sticker,
                                    events::RoomEvent<msgs::Redacted>,
                                    events::RoomEvent<msgs::Audio>,
                                    events::RoomEvent<msgs::Emote>,
                                    events::RoomEvent<msgs::File>,
                                    events::RoomEvent<msgs::Image>,
                                    // TODO: events::RoomEvent<msgs::Location>,
                                    events::RoomEvent<msgs::Notice>,
                                    events::RoomEvent<msgs::Text>,
                                    events::RoomEvent<msgs::Video>>;

struct TimelineEvent
{
        TimelineEvents data;
};

inline void
from_json(const json &obj, TimelineEvent &e)
{
        const auto type = mtx::events::getEventType(obj);
        using namespace mtx::events::state;
        using namespace mtx::events::msg;

        switch (type) {
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
                                std::cout << "Invalid event type: " << err.what() << " "
                                          << obj.dump(2) << '\n';
                                return;
                        }

                        std::cout << "Invalid event type: " << obj.dump(2) << '\n';
                        break;
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
                case MsgType::Unknown:
                        return;
                }
                break;
        }
        case events::EventType::Sticker: {
                e.data = events::Sticker(obj);
                break;
        }
        case events::EventType::RoomPinnedEvents:
        case events::EventType::RoomKeyRequest: // Not part of the timeline
        case events::EventType::Tag:            // Not part of the timeline
        case events::EventType::KeyVerificationCancel:
        case events::EventType::KeyVerificationRequest:
        case events::EventType::KeyVerificationStart:
        case events::EventType::KeyVerificationAccept:
        case events::EventType::KeyVerificationKey:
        case events::EventType::KeyVerificationMac:
        case events::EventType::Unsupported:
                return;
        }
}
} // namespace collections
} // namespace events
} // namespace mtx
