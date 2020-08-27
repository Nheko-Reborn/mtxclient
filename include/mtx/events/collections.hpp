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
#include "mtx/events/nheko_extensions/hidden_events.hpp"
#include "mtx/events/pinned_events.hpp"
#include "mtx/events/power_levels.hpp"
#include "mtx/events/presence.hpp"
#include "mtx/events/reaction.hpp"
#include "mtx/events/redaction.hpp"
#include "mtx/events/tag.hpp"
#include "mtx/events/tombstone.hpp"
#include "mtx/events/topic.hpp"
#include "mtx/events/voip.hpp"
#include "mtx/pushrules.hpp"

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

//! Collection of key verification events
using DeviceEvents = std::variant<events::DeviceEvent<msgs::RoomKey>,
                                  events::DeviceEvent<msgs::KeyRequest>,
                                  events::DeviceEvent<msgs::OlmEncrypted>,
                                  events::DeviceEvent<msgs::Encrypted>,
                                  events::DeviceEvent<msgs::KeyVerificationRequest>,
                                  events::DeviceEvent<msgs::KeyVerificationStart>,
                                  events::DeviceEvent<msgs::KeyVerificationReady>,
                                  events::DeviceEvent<msgs::KeyVerificationDone>,
                                  events::DeviceEvent<msgs::KeyVerificationAccept>,
                                  events::DeviceEvent<msgs::KeyVerificationCancel>,
                                  events::DeviceEvent<msgs::KeyVerificationKey>,
                                  events::DeviceEvent<msgs::KeyVerificationMac>>;

//! Collection of room specific account data
using RoomAccountDataEvents =
  std::variant<events::Event<account_data::Tags>,
               events::Event<pushrules::GlobalRuleset>,
               events::Event<account_data::nheko_extensions::HiddenEvents>>;

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
                                    events::RoomEvent<msgs::Reaction>,
                                    events::RoomEvent<msgs::Redacted>,
                                    events::RoomEvent<msgs::Audio>,
                                    events::RoomEvent<msgs::Emote>,
                                    events::RoomEvent<msgs::File>,
                                    events::RoomEvent<msgs::Image>,
                                    // TODO: events::RoomEvent<msgs::Location>,
                                    events::RoomEvent<msgs::Notice>,
                                    events::RoomEvent<msgs::Text>,
                                    events::RoomEvent<msgs::Video>,
                                    events::RoomEvent<msgs::CallInvite>,
                                    events::RoomEvent<msgs::CallCandidates>,
                                    events::RoomEvent<msgs::CallAnswer>,
                                    events::RoomEvent<msgs::CallHangUp>>;

struct TimelineEvent
{
        TimelineEvents data;
};

void
from_json(const json &obj, TimelineEvent &e);

} // namespace collections

template<typename Content>
constexpr EventType message_content_to_type = EventType::Unsupported;

template<>
constexpr EventType message_content_to_type<mtx::events::msg::Encrypted> = EventType::RoomEncrypted;
template<>
constexpr EventType message_content_to_type<mtx::events::msg::Reaction> = EventType::Reaction;
template<>
constexpr EventType message_content_to_type<mtx::events::msg::Audio> = EventType::RoomMessage;
template<>
constexpr EventType message_content_to_type<mtx::events::msg::Emote> = EventType::RoomMessage;
template<>
constexpr EventType message_content_to_type<mtx::events::msg::File> = EventType::RoomMessage;
template<>
constexpr EventType message_content_to_type<mtx::events::msg::Image> = EventType::RoomMessage;
template<>
constexpr EventType message_content_to_type<mtx::events::msg::Notice> = EventType::RoomMessage;
template<>
constexpr EventType message_content_to_type<mtx::events::msg::Text> = EventType::RoomMessage;
template<>
constexpr EventType message_content_to_type<mtx::events::msg::Video> = EventType::RoomMessage;
template<>
constexpr EventType message_content_to_type<mtx::events::msg::StickerImage> = EventType::Sticker;

template<typename Content>
constexpr EventType state_content_to_type = EventType::Unsupported;

template<>
constexpr EventType state_content_to_type<mtx::events::state::Aliases> = EventType::RoomAliases;
template<>
constexpr EventType state_content_to_type<mtx::events::state::Avatar> = EventType::RoomAvatar;
template<>
constexpr EventType state_content_to_type<mtx::events::state::CanonicalAlias> =
  EventType::RoomCanonicalAlias;
template<>
constexpr EventType state_content_to_type<mtx::events::state::Create> = EventType::RoomCreate;
template<>
constexpr EventType state_content_to_type<mtx::events::state::Encryption> =
  EventType::RoomEncryption;
template<>
constexpr EventType state_content_to_type<mtx::events::state::GuestAccess> =
  EventType::RoomGuestAccess;
template<>
constexpr EventType state_content_to_type<mtx::events::state::HistoryVisibility> =
  EventType::RoomHistoryVisibility;
template<>
constexpr EventType state_content_to_type<mtx::events::state::JoinRules> = EventType::RoomJoinRules;
template<>
constexpr EventType state_content_to_type<mtx::events::state::Member> = EventType::RoomMember;
template<>
constexpr EventType state_content_to_type<mtx::events::state::Name> = EventType::RoomName;
template<>
constexpr EventType state_content_to_type<mtx::events::state::PinnedEvents> =
  EventType::RoomPinnedEvents;
template<>
constexpr EventType state_content_to_type<mtx::events::state::PowerLevels> =
  EventType::RoomPowerLevels;
template<>
constexpr EventType state_content_to_type<mtx::events::state::Tombstone> = EventType::RoomTombstone;
template<>
constexpr EventType state_content_to_type<mtx::events::state::Topic> = EventType::RoomTopic;

template<typename Content>
constexpr EventType to_device_content_to_type = EventType::Unsupported;

template<>
constexpr EventType to_device_content_to_type<mtx::events::msg::RoomKey> = EventType::RoomKey;
template<>
constexpr EventType to_device_content_to_type<mtx::events::msg::KeyRequest> =
  EventType::RoomKeyRequest;
// template<>
// constexpr EventType to_device_content_to_type<mtx::events::msg::OlmEncrypted> = EventType::;Olm;
template<>
constexpr EventType to_device_content_to_type<mtx::events::msg::Encrypted> =
  EventType::RoomEncrypted;
template<>
constexpr EventType to_device_content_to_type<mtx::events::msg::KeyVerificationRequest> =
  EventType::KeyVerificationRequest;
template<>
constexpr EventType to_device_content_to_type<mtx::events::msg::KeyVerificationStart> =
  EventType::KeyVerificationStart;
template<>
constexpr EventType to_device_content_to_type<mtx::events::msg::KeyVerificationReady> =
  EventType::KeyVerificationReady;
template<>
constexpr EventType to_device_content_to_type<mtx::events::msg::KeyVerificationDone> =
  EventType::KeyVerificationDone;
template<>
constexpr EventType to_device_content_to_type<mtx::events::msg::KeyVerificationAccept> =
  EventType::KeyVerificationAccept;
template<>
constexpr EventType to_device_content_to_type<mtx::events::msg::KeyVerificationCancel> =
  EventType::KeyVerificationCancel;
template<>
constexpr EventType to_device_content_to_type<mtx::events::msg::KeyVerificationKey> =
  EventType::KeyVerificationKey;
template<>
constexpr EventType to_device_content_to_type<mtx::events::msg::KeyVerificationMac> =
  EventType::KeyVerificationMac;
} // namespace events
} // namespace mtx
