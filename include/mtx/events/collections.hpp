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
#include "mtx/events/reaction.hpp"
#include "mtx/events/redaction.hpp"
#include "mtx/events/tag.hpp"
#include "mtx/events/tombstone.hpp"
#include "mtx/events/topic.hpp"
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
                                  events::DeviceEvent<msgs::KeyVerificationAccept>,
                                  events::DeviceEvent<msgs::KeyVerificationCancel>,
                                  events::DeviceEvent<msgs::KeyVerificationKey>,
                                  events::DeviceEvent<msgs::KeyVerificationMac>>;

//! Collection of room specific account data
using RoomAccountDataEvents =
  std::variant<events::Event<account_data::Tag>, events::Event<pushrules::GlobalRuleset>>;

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
                                    events::RoomEvent<msgs::Video>>;

struct TimelineEvent
{
        TimelineEvents data;
};

void
from_json(const json &obj, TimelineEvent &e);

} // namespace collections
} // namespace events
} // namespace mtx
