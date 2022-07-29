#pragma once

/// @file
/// @brief Collections to store multiple events of different types

#include <variant>

#include "mtx/events.hpp"
#include "mtx/events/account_data/direct.hpp"
#include "mtx/events/account_data/fully_read.hpp"
#include "mtx/events/aliases.hpp"
#include "mtx/events/avatar.hpp"
#include "mtx/events/canonical_alias.hpp"
#include "mtx/events/create.hpp"
#include "mtx/events/encrypted.hpp"
#include "mtx/events/encryption.hpp"
#include "mtx/events/ephemeral/receipt.hpp"
#include "mtx/events/ephemeral/typing.hpp"
#include "mtx/events/guest_access.hpp"
#include "mtx/events/history_visibility.hpp"
#include "mtx/events/join_rules.hpp"
#include "mtx/events/member.hpp"
#include "mtx/events/mscs/image_packs.hpp"
#include "mtx/events/name.hpp"
#include "mtx/events/nheko_extensions/hidden_events.hpp"
#include "mtx/events/pinned_events.hpp"
#include "mtx/events/policy_rules.hpp"
#include "mtx/events/power_levels.hpp"
#include "mtx/events/presence.hpp"
#include "mtx/events/reaction.hpp"
#include "mtx/events/redaction.hpp"
#include "mtx/events/spaces.hpp"
#include "mtx/events/tag.hpp"
#include "mtx/events/tombstone.hpp"
#include "mtx/events/topic.hpp"
#include "mtx/events/unknown.hpp"
#include "mtx/events/voip.hpp"
#include "mtx/events/widget.hpp"
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

//! Collection of key verification events
using DeviceEvents =
  std::variant<mtx::events::DeviceEvent<mtx::events::msg::RoomKey>,
               mtx::events::DeviceEvent<mtx::events::msg::ForwardedRoomKey>,
               mtx::events::DeviceEvent<mtx::events::msg::KeyRequest>,
               mtx::events::DeviceEvent<mtx::events::msg::OlmEncrypted>,
               mtx::events::DeviceEvent<mtx::events::msg::Encrypted>,
               mtx::events::DeviceEvent<mtx::events::msg::Dummy>,
               mtx::events::DeviceEvent<mtx::events::msg::KeyVerificationRequest>,
               mtx::events::DeviceEvent<mtx::events::msg::KeyVerificationStart>,
               mtx::events::DeviceEvent<mtx::events::msg::KeyVerificationReady>,
               mtx::events::DeviceEvent<mtx::events::msg::KeyVerificationDone>,
               mtx::events::DeviceEvent<mtx::events::msg::KeyVerificationAccept>,
               mtx::events::DeviceEvent<mtx::events::msg::KeyVerificationCancel>,
               mtx::events::DeviceEvent<mtx::events::msg::KeyVerificationKey>,
               mtx::events::DeviceEvent<mtx::events::msg::KeyVerificationMac>,
               mtx::events::DeviceEvent<mtx::events::msg::SecretRequest>,
               mtx::events::DeviceEvent<mtx::events::msg::SecretSend>,
               mtx::events::DeviceEvent<mtx::events::Unknown>>;

//! Collection of room specific account data
using RoomAccountDataEvents = std::variant<
  mtx::events::AccountDataEvent<mtx::events::account_data::Tags>,
  mtx::events::AccountDataEvent<mtx::events::account_data::Direct>,
  mtx::events::AccountDataEvent<mtx::events::account_data::FullyRead>,
  mtx::events::AccountDataEvent<mtx::pushrules::GlobalRuleset>,
  mtx::events::AccountDataEvent<mtx::events::account_data::nheko_extensions::HiddenEvents>,
  mtx::events::AccountDataEvent<mtx::events::msc2545::ImagePack>,
  mtx::events::AccountDataEvent<mtx::events::msc2545::ImagePackRooms>,
  mtx::events::AccountDataEvent<mtx::events::Unknown>>;

//! Collection of @p StateEvent only.
using StateEvents =
  std::variant<mtx::events::StateEvent<mtx::events::state::Aliases>,
               mtx::events::StateEvent<mtx::events::state::Avatar>,
               mtx::events::StateEvent<mtx::events::state::CanonicalAlias>,
               mtx::events::StateEvent<mtx::events::state::Create>,
               mtx::events::StateEvent<mtx::events::state::Encryption>,
               mtx::events::StateEvent<mtx::events::state::GuestAccess>,
               mtx::events::StateEvent<mtx::events::state::HistoryVisibility>,
               mtx::events::StateEvent<mtx::events::state::JoinRules>,
               mtx::events::StateEvent<mtx::events::state::Member>,
               mtx::events::StateEvent<mtx::events::state::Name>,
               mtx::events::StateEvent<mtx::events::state::PinnedEvents>,
               mtx::events::StateEvent<mtx::events::state::PowerLevels>,
               mtx::events::StateEvent<mtx::events::state::policy_rule::UserRule>,
               mtx::events::StateEvent<mtx::events::state::policy_rule::RoomRule>,
               mtx::events::StateEvent<mtx::events::state::policy_rule::ServerRule>,
               mtx::events::StateEvent<mtx::events::state::space::Child>,
               mtx::events::StateEvent<mtx::events::state::space::Parent>,
               mtx::events::StateEvent<mtx::events::state::Tombstone>,
               mtx::events::StateEvent<mtx::events::state::Topic>,
               mtx::events::StateEvent<mtx::events::state::Widget>,
               mtx::events::StateEvent<mtx::events::msg::Redacted>,
               mtx::events::StateEvent<mtx::events::msc2545::ImagePack>,
               mtx::events::StateEvent<mtx::events::Unknown>>;

//! Collection of @p StrippedEvent only.
using StrippedEvents =
  std::variant<mtx::events::StrippedEvent<mtx::events::state::Aliases>,
               mtx::events::StrippedEvent<mtx::events::state::Avatar>,
               mtx::events::StrippedEvent<mtx::events::state::CanonicalAlias>,
               mtx::events::StrippedEvent<mtx::events::state::Create>,
               mtx::events::StrippedEvent<mtx::events::state::Encryption>,
               mtx::events::StrippedEvent<mtx::events::state::GuestAccess>,
               mtx::events::StrippedEvent<mtx::events::state::HistoryVisibility>,
               mtx::events::StrippedEvent<mtx::events::state::JoinRules>,
               mtx::events::StrippedEvent<mtx::events::state::Member>,
               mtx::events::StrippedEvent<mtx::events::state::Name>,
               mtx::events::StrippedEvent<mtx::events::state::PinnedEvents>,
               mtx::events::StrippedEvent<mtx::events::state::PowerLevels>,
               mtx::events::StrippedEvent<mtx::events::state::policy_rule::UserRule>,
               mtx::events::StrippedEvent<mtx::events::state::policy_rule::RoomRule>,
               mtx::events::StrippedEvent<mtx::events::state::policy_rule::ServerRule>,
               mtx::events::StrippedEvent<mtx::events::state::space::Child>,
               mtx::events::StrippedEvent<mtx::events::state::space::Parent>,
               mtx::events::StrippedEvent<mtx::events::state::Tombstone>,
               mtx::events::StrippedEvent<mtx::events::state::Topic>,
               mtx::events::StrippedEvent<mtx::events::state::Widget>,
               mtx::events::StrippedEvent<mtx::events::msg::Redacted>,
               mtx::events::StrippedEvent<mtx::events::Unknown>>;

//! Collection of @p StateEvent and @p RoomEvent. Those events would be
//! available on the returned timeline.
using TimelineEvents =
  std::variant<mtx::events::StateEvent<mtx::events::state::Aliases>,
               mtx::events::StateEvent<mtx::events::state::Avatar>,
               mtx::events::StateEvent<mtx::events::state::CanonicalAlias>,
               mtx::events::StateEvent<mtx::events::state::Create>,
               mtx::events::StateEvent<mtx::events::state::Encryption>,
               mtx::events::StateEvent<mtx::events::state::GuestAccess>,
               mtx::events::StateEvent<mtx::events::state::HistoryVisibility>,
               mtx::events::StateEvent<mtx::events::state::JoinRules>,
               mtx::events::StateEvent<mtx::events::state::Member>,
               mtx::events::StateEvent<mtx::events::state::Name>,
               mtx::events::StateEvent<mtx::events::state::PinnedEvents>,
               mtx::events::StateEvent<mtx::events::state::PowerLevels>,
               mtx::events::StateEvent<mtx::events::state::policy_rule::UserRule>,
               mtx::events::StateEvent<mtx::events::state::policy_rule::RoomRule>,
               mtx::events::StateEvent<mtx::events::state::policy_rule::ServerRule>,
               mtx::events::StateEvent<mtx::events::state::space::Child>,
               mtx::events::StateEvent<mtx::events::state::space::Parent>,
               mtx::events::StateEvent<mtx::events::state::Tombstone>,
               mtx::events::StateEvent<mtx::events::state::Topic>,
               mtx::events::StateEvent<mtx::events::state::Widget>,
               mtx::events::StateEvent<mtx::events::msc2545::ImagePack>,
               mtx::events::StateEvent<mtx::events::msg::Redacted>,
               mtx::events::EncryptedEvent<mtx::events::msg::Encrypted>,
               mtx::events::RedactionEvent<mtx::events::msg::Redaction>,
               mtx::events::Sticker,
               mtx::events::RoomEvent<mtx::events::msg::Reaction>,
               mtx::events::RoomEvent<mtx::events::msg::Redacted>,
               mtx::events::RoomEvent<mtx::events::msg::Audio>,
               mtx::events::RoomEvent<mtx::events::msg::Emote>,
               mtx::events::RoomEvent<mtx::events::msg::File>,
               mtx::events::RoomEvent<mtx::events::msg::Image>,
               // TODO: events::RoomEvent<mtx::events::msg::Location>,
               mtx::events::RoomEvent<mtx::events::msg::Notice>,
               mtx::events::RoomEvent<mtx::events::msg::Text>,
               mtx::events::RoomEvent<mtx::events::msg::Video>,
               mtx::events::RoomEvent<mtx::events::msg::KeyVerificationRequest>,
               mtx::events::RoomEvent<mtx::events::msg::KeyVerificationStart>,
               mtx::events::RoomEvent<mtx::events::msg::KeyVerificationReady>,
               mtx::events::RoomEvent<mtx::events::msg::KeyVerificationDone>,
               mtx::events::RoomEvent<mtx::events::msg::KeyVerificationAccept>,
               mtx::events::RoomEvent<mtx::events::msg::KeyVerificationCancel>,
               mtx::events::RoomEvent<mtx::events::msg::KeyVerificationKey>,
               mtx::events::RoomEvent<mtx::events::msg::KeyVerificationMac>,
               mtx::events::RoomEvent<mtx::events::voip::CallCandidates>,
               mtx::events::RoomEvent<mtx::events::voip::CallInvite>,
               mtx::events::RoomEvent<mtx::events::voip::CallAnswer>,
               mtx::events::RoomEvent<mtx::events::voip::CallHangUp>,
               mtx::events::RoomEvent<mtx::events::voip::CallSelectAnswer>,
               mtx::events::RoomEvent<mtx::events::voip::CallReject>,
               mtx::events::RoomEvent<mtx::events::voip::CallNegotiate>,
               mtx::events::RoomEvent<mtx::events::Unknown>>;

using EphemeralEvents = std::variant<mtx::events::EphemeralEvent<mtx::events::ephemeral::Typing>,
                                     mtx::events::EphemeralEvent<mtx::events::ephemeral::Receipt>,
                                     mtx::events::EphemeralEvent<mtx::events::Unknown>>;

//! A wapper around TimelineEvent, that produces less noisy compiler errors.
struct TimelineEvent
{
    TimelineEvents data;

    friend void from_json(const nlohmann::json &obj, TimelineEvent &e);
};

} // namespace collections

//! Get the right event type for some type of message content.
template<typename Content>
constexpr inline EventType message_content_to_type = EventType::Unsupported;

template<>
constexpr inline EventType message_content_to_type<mtx::events::msg::Encrypted> =
  EventType::RoomEncrypted;
template<>
constexpr inline EventType message_content_to_type<mtx::events::msg::Reaction> =
  EventType::Reaction;
template<>
constexpr inline EventType message_content_to_type<mtx::events::msg::Audio> =
  EventType::RoomMessage;
template<>
constexpr inline EventType message_content_to_type<mtx::events::msg::Emote> =
  EventType::RoomMessage;
template<>
constexpr inline EventType message_content_to_type<mtx::events::msg::File> = EventType::RoomMessage;
template<>
constexpr inline EventType message_content_to_type<mtx::events::msg::Image> =
  EventType::RoomMessage;
template<>
constexpr inline EventType message_content_to_type<mtx::events::msg::Notice> =
  EventType::RoomMessage;
template<>
constexpr inline EventType message_content_to_type<mtx::events::msg::Text> = EventType::RoomMessage;
template<>
constexpr inline EventType message_content_to_type<mtx::events::msg::Video> =
  EventType::RoomMessage;
template<>
constexpr inline EventType message_content_to_type<mtx::events::msg::StickerImage> =
  EventType::Sticker;
template<>
constexpr inline EventType message_content_to_type<mtx::events::voip::CallInvite> =
  EventType::CallInvite;
template<>
constexpr inline EventType message_content_to_type<mtx::events::voip::CallCandidates> =
  EventType::CallCandidates;
template<>
constexpr inline EventType message_content_to_type<mtx::events::voip::CallAnswer> =
  EventType::CallAnswer;
template<>
constexpr inline EventType message_content_to_type<mtx::events::voip::CallHangUp> =
  EventType::CallHangUp;
template<>
constexpr inline EventType message_content_to_type<mtx::events::voip::CallSelectAnswer> =
  EventType::CallSelectAnswer;
template<>
constexpr inline EventType message_content_to_type<mtx::events::voip::CallReject> =
  EventType::CallReject;
template<>
constexpr inline EventType message_content_to_type<mtx::events::voip::CallNegotiate> =
  EventType::CallNegotiate;

//! Get the right event type for some type of state event content.
template<typename Content>
constexpr inline EventType state_content_to_type = EventType::Unsupported;

template<>
constexpr inline EventType state_content_to_type<mtx::events::state::Aliases> =
  EventType::RoomAliases;
template<>
constexpr inline EventType state_content_to_type<mtx::events::state::Avatar> =
  EventType::RoomAvatar;
template<>
constexpr inline EventType state_content_to_type<mtx::events::state::CanonicalAlias> =
  EventType::RoomCanonicalAlias;
template<>
constexpr inline EventType state_content_to_type<mtx::events::state::Create> =
  EventType::RoomCreate;
template<>
constexpr inline EventType state_content_to_type<mtx::events::state::Encryption> =
  EventType::RoomEncryption;
template<>
constexpr inline EventType state_content_to_type<mtx::events::state::GuestAccess> =
  EventType::RoomGuestAccess;
template<>
constexpr inline EventType state_content_to_type<mtx::events::state::HistoryVisibility> =
  EventType::RoomHistoryVisibility;
template<>
constexpr inline EventType state_content_to_type<mtx::events::state::JoinRules> =
  EventType::RoomJoinRules;
template<>
constexpr inline EventType state_content_to_type<mtx::events::state::Member> =
  EventType::RoomMember;
template<>
constexpr inline EventType state_content_to_type<mtx::events::state::Name> = EventType::RoomName;
template<>
constexpr inline EventType state_content_to_type<mtx::events::state::PinnedEvents> =
  EventType::RoomPinnedEvents;
template<>
constexpr inline EventType state_content_to_type<mtx::events::state::policy_rule::UserRule> =
  EventType::PolicyRuleUser;
template<>
constexpr inline EventType state_content_to_type<mtx::events::state::policy_rule::RoomRule> =
  EventType::PolicyRuleRoom;
template<>
constexpr inline EventType state_content_to_type<mtx::events::state::policy_rule::ServerRule> =
  EventType::PolicyRuleServer;
template<>
constexpr inline EventType state_content_to_type<mtx::events::state::PowerLevels> =
  EventType::RoomPowerLevels;
template<>
constexpr inline EventType state_content_to_type<mtx::events::state::Tombstone> =
  EventType::RoomTombstone;

template<>
constexpr inline EventType state_content_to_type<mtx::events::state::space::Child> =
  EventType::SpaceChild;
template<>
constexpr inline EventType state_content_to_type<mtx::events::state::space::Parent> =
  EventType::SpaceParent;

template<>
constexpr inline EventType state_content_to_type<mtx::events::state::Widget> =
  EventType::VectorWidget;

template<>
constexpr inline EventType state_content_to_type<mtx::events::state::Topic> = EventType::RoomTopic;
template<>
constexpr inline EventType state_content_to_type<mtx::events::msc2545::ImagePack> =
  EventType::ImagePackInRoom;

//! Get the right event type for some type of device message content.
template<typename Content>
constexpr inline EventType to_device_content_to_type = EventType::Unsupported;

template<>
constexpr inline EventType to_device_content_to_type<mtx::events::msg::RoomKey> =
  EventType::RoomKey;
template<>
constexpr inline EventType to_device_content_to_type<mtx::events::msg::ForwardedRoomKey> =
  EventType::ForwardedRoomKey;
template<>
constexpr inline EventType to_device_content_to_type<mtx::events::msg::KeyRequest> =
  EventType::RoomKeyRequest;
template<>
constexpr inline EventType to_device_content_to_type<mtx::events::msg::OlmEncrypted> =
  EventType::RoomEncrypted;
template<>
constexpr inline EventType to_device_content_to_type<mtx::events::msg::Encrypted> =
  EventType::RoomEncrypted;
template<>
constexpr inline EventType to_device_content_to_type<mtx::events::msg::Dummy> = EventType::Dummy;
template<>
constexpr inline EventType to_device_content_to_type<mtx::events::msg::KeyVerificationRequest> =
  EventType::KeyVerificationRequest;
template<>
constexpr inline EventType to_device_content_to_type<mtx::events::msg::KeyVerificationStart> =
  EventType::KeyVerificationStart;
template<>
constexpr inline EventType to_device_content_to_type<mtx::events::msg::KeyVerificationReady> =
  EventType::KeyVerificationReady;
template<>
constexpr inline EventType to_device_content_to_type<mtx::events::msg::KeyVerificationDone> =
  EventType::KeyVerificationDone;
template<>
constexpr inline EventType to_device_content_to_type<mtx::events::msg::KeyVerificationAccept> =
  EventType::KeyVerificationAccept;
template<>
constexpr inline EventType to_device_content_to_type<mtx::events::msg::KeyVerificationCancel> =
  EventType::KeyVerificationCancel;
template<>
constexpr inline EventType to_device_content_to_type<mtx::events::msg::KeyVerificationKey> =
  EventType::KeyVerificationKey;
template<>
constexpr inline EventType to_device_content_to_type<mtx::events::msg::KeyVerificationMac> =
  EventType::KeyVerificationMac;
template<>
constexpr inline EventType to_device_content_to_type<mtx::events::msg::SecretSend> =
  EventType::SecretSend;
template<>
constexpr inline EventType to_device_content_to_type<mtx::events::msg::SecretRequest> =
  EventType::SecretRequest;

//! Get the right event type for some type of account_data event content.
template<typename Content>
constexpr inline EventType account_data_content_to_type = EventType::Unsupported;

template<>
constexpr inline EventType account_data_content_to_type<mtx::events::msc2545::ImagePack> =
  EventType::ImagePackInAccountData;
template<>
constexpr inline EventType account_data_content_to_type<mtx::events::msc2545::ImagePackRooms> =
  EventType::ImagePackRooms;
template<>
constexpr inline EventType account_data_content_to_type<mtx::events::account_data::Tags> =
  EventType::Tag;
template<>
constexpr inline EventType account_data_content_to_type<mtx::events::account_data::Direct> =
  EventType::Direct;
template<>
constexpr inline EventType
  account_data_content_to_type<mtx::events::account_data::nheko_extensions::HiddenEvents> =
    EventType::NhekoHiddenEvents;

} // namespace events
} // namespace mtx
