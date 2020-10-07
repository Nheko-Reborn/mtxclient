#include "mtx/responses/common.hpp"

#include "mtx/events.hpp"
#include "mtx/events/aliases.hpp"
#include "mtx/events/avatar.hpp"
#include "mtx/events/canonical_alias.hpp"
#include "mtx/events/create.hpp"
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
#include "mtx/events/topic.hpp"
#include "mtx/events/voip.hpp"

#include <iostream>

using json = nlohmann::json;
using namespace mtx::events::account_data;
using namespace mtx::events::state;
using namespace mtx::events::msg;

namespace mtx {
namespace responses {
void
from_json(const nlohmann::json &obj, GroupId &response)
{
        response.group_id = obj.at("group_id");
}

void
from_json(const nlohmann::json &obj, EventId &response)
{
        response.event_id = obj.at("event_id");
}

void
from_json(const nlohmann::json &obj, FilterId &response)
{
        response.filter_id = obj.at("filter_id");
}

namespace utils {

void
log_error(json::exception &err, const json &event)
{
        std::cout << err.what() << std::endl;
        std::cout << event.dump(2) << std::endl;
}

void
log_error(std::string err, const json &event)
{
        std::cout << err << std::endl;
        std::cout << event.dump(2) << std::endl;
}

void
parse_room_account_data_events(
  const json &events,
  std::vector<mtx::events::collections::RoomAccountDataEvents> &container)
{
        container.clear();
        container.reserve(events.size());

        for (const auto &e : events) {
                const auto type = mtx::events::getEventType(e);

                switch (type) {
                case events::EventType::Tag: {
                        try {
                                container.emplace_back(events::Event<Tags>(e));
                        } catch (json::exception &err) {
                                log_error(err, e);
                        }
                        break;
                }
                case events::EventType::PushRules: {
                        try {
                                container.emplace_back(events::Event<pushrules::GlobalRuleset>(e));
                        } catch (json::exception &err) {
                                log_error(err, e);
                        }
                        break;
                }
                case events::EventType::NhekoHiddenEvents: {
                        try {
                                container.emplace_back(
                                  events::Event<nheko_extensions::HiddenEvents>(e));
                        } catch (json::exception &err) {
                                log_error(err, e);
                        }
                        break;
                }
                case events::EventType::KeyVerificationCancel:
                case events::EventType::KeyVerificationRequest:
                case events::EventType::KeyVerificationStart:
                case events::EventType::KeyVerificationReady:
                case events::EventType::KeyVerificationDone:
                case events::EventType::KeyVerificationAccept:
                case events::EventType::KeyVerificationKey:
                case events::EventType::KeyVerificationMac:
                case events::EventType::Presence:
                case events::EventType::Reaction:
                case events::EventType::RoomAliases:
                case events::EventType::RoomAvatar:
                case events::EventType::RoomCanonicalAlias:
                case events::EventType::RoomCreate:
                case events::EventType::RoomEncrypted:
                case events::EventType::RoomEncryption:
                case events::EventType::RoomGuestAccess:
                case events::EventType::RoomHistoryVisibility:
                case events::EventType::RoomJoinRules:
                case events::EventType::RoomKey:
                case events::EventType::ForwardedRoomKey:
                case events::EventType::RoomKeyRequest:
                case events::EventType::RoomMember:
                case events::EventType::RoomMessage:
                case events::EventType::RoomName:
                case events::EventType::RoomPinnedEvents:
                case events::EventType::RoomPowerLevels:
                case events::EventType::RoomRedaction:
                case events::EventType::RoomTombstone:
                case events::EventType::RoomTopic:
                case events::EventType::Sticker:
                case events::EventType::CallInvite:
                case events::EventType::CallCandidates:
                case events::EventType::CallAnswer:
                case events::EventType::CallHangUp:
                case events::EventType::Unsupported:
                        continue;
                }
        }
}

void
compose_timeline_events(json &events,
                        const std::vector<mtx::events::collections::TimelineEvents> &container)
{
        const auto c = container.at(0);
        events       = std::visit([](auto e) { return json(e); }, c);
}

void
parse_timeline_events(const json &events,
                      std::vector<mtx::events::collections::TimelineEvents> &container)
{
        container.clear();
        container.reserve(events.size());

        for (const auto &e : events) {
                const auto type = mtx::events::getEventType(e);

                switch (type) {
                case events::EventType::Reaction: {
                        try {
                                container.emplace_back(events::RoomEvent<events::msg::Reaction>(e));
                        } catch (json::exception &err) {
                                log_error(err, e);
                        }

                        break;
                }
                case events::EventType::RoomAliases: {
                        try {
                                container.emplace_back(events::StateEvent<Aliases>(e));
                        } catch (json::exception &err) {
                                log_error(err, e);
                        }

                        break;
                }
                case events::EventType::RoomAvatar: {
                        try {
                                container.emplace_back(events::StateEvent<Avatar>(e));
                        } catch (json::exception &err) {
                                log_error(err, e);
                        }

                        break;
                }
                case events::EventType::RoomCanonicalAlias: {
                        try {
                                container.emplace_back(events::StateEvent<CanonicalAlias>(e));
                        } catch (json::exception &err) {
                                log_error(err, e);
                        }

                        break;
                }
                case events::EventType::RoomCreate: {
                        try {
                                container.emplace_back(events::StateEvent<Create>(e));
                        } catch (json::exception &err) {
                                log_error(err, e);
                        }

                        break;
                }
                case events::EventType::RoomEncrypted: {
                        try {
                                container.emplace_back(
                                  events::EncryptedEvent<mtx::events::msg::Encrypted>(e));
                        } catch (json::exception &err) {
                                log_error(err, e);
                        }

                        break;
                }
                case events::EventType::RoomEncryption: {
                        try {
                                container.emplace_back(events::StateEvent<Encryption>(e));
                        } catch (json::exception &err) {
                                log_error(err, e);
                        }

                        break;
                }
                case events::EventType::RoomGuestAccess: {
                        try {
                                container.emplace_back(events::StateEvent<GuestAccess>(e));
                        } catch (json::exception &err) {
                                log_error(err, e);
                        }

                        break;
                }
                case events::EventType::RoomHistoryVisibility: {
                        try {
                                container.emplace_back(events::StateEvent<HistoryVisibility>(e));
                        } catch (json::exception &err) {
                                log_error(err, e);
                        }

                        break;
                }
                case events::EventType::RoomJoinRules: {
                        try {
                                container.emplace_back(events::StateEvent<JoinRules>(e));
                        } catch (json::exception &err) {
                                log_error(err, e);
                        }

                        break;
                }
                case events::EventType::RoomMember: {
                        try {
                                container.emplace_back(events::StateEvent<Member>(e));
                        } catch (json::exception &err) {
                                log_error(err, e);
                        }

                        break;
                }
                case events::EventType::RoomName: {
                        try {
                                container.emplace_back(events::StateEvent<Name>(e));
                        } catch (json::exception &err) {
                                log_error(err, e);
                        }

                        break;
                }
                case events::EventType::RoomPowerLevels: {
                        try {
                                container.emplace_back(events::StateEvent<PowerLevels>(e));
                        } catch (json::exception &err) {
                                log_error(err, e);
                        }

                        break;
                }
                case events::EventType::RoomRedaction: {
                        try {
                                container.emplace_back(
                                  events::RedactionEvent<mtx::events::msg::Redaction>(e));
                        } catch (json::exception &err) {
                                log_error(err, e);
                        }

                        break;
                }
                case events::EventType::RoomTombstone: {
                        try {
                                container.emplace_back(events::StateEvent<Tombstone>(e));
                        } catch (json::exception &err) {
                                log_error(err, e);
                        }

                        break;
                }
                case events::EventType::RoomTopic: {
                        try {
                                container.emplace_back(events::StateEvent<Topic>(e));
                        } catch (json::exception &err) {
                                log_error(err, e);
                        }

                        break;
                }
                case events::EventType::KeyVerificationStart: {
                        try {
                                container.emplace_back(
                                  events::RoomEvent<events::msg::KeyVerificationStart>(e));
                        } catch (json::exception &err) {
                                log_error(err, e);
                        }

                        break;
                }
                case events::EventType::KeyVerificationAccept: {
                        try {
                                container.emplace_back(
                                  events::RoomEvent<events::msg::KeyVerificationAccept>(e));
                        } catch (json::exception &err) {
                                log_error(err, e);
                        }

                        break;
                }
                case events::EventType::KeyVerificationDone: {
                        try {
                                container.emplace_back(
                                  events::RoomEvent<events::msg::KeyVerificationDone>(e));
                        } catch (json::exception &err) {
                                log_error(err, e);
                        }

                        break;
                }
                case events::EventType::KeyVerificationReady: {
                        try {
                                container.emplace_back(
                                  events::RoomEvent<events::msg::KeyVerificationReady>(e));
                        } catch (json::exception &err) {
                                log_error(err, e);
                        }

                        break;
                }
                case events::EventType::KeyVerificationKey: {
                        try {
                                container.emplace_back(
                                  events::RoomEvent<events::msg::KeyVerificationKey>(e));
                        } catch (json::exception &err) {
                                log_error(err, e);
                        }

                        break;
                }
                case events::EventType::KeyVerificationMac: {
                        try {
                                container.emplace_back(
                                  events::RoomEvent<events::msg::KeyVerificationMac>(e));
                        } catch (json::exception &err) {
                                log_error(err, e);
                        }

                        break;
                }
                case events::EventType::KeyVerificationCancel: {
                        try {
                                container.emplace_back(
                                  events::RoomEvent<events::msg::KeyVerificationCancel>(e));
                        } catch (json::exception &err) {
                                log_error(err, e);
                        }

                        break;
                }
                case events::EventType::RoomMessage: {
                        using MsgType       = mtx::events::MessageType;
                        const auto msg_type = mtx::events::getMessageType(e.at("content"));

                        if (msg_type == events::MessageType::Unknown) {
                                try {
                                        auto unsigned_data =
                                          e.at("unsigned").at("redacted_by").get<std::string>();

                                        if (unsigned_data.empty())
                                                continue;

                                        container.emplace_back(
                                          events::RoomEvent<events::msg::Redacted>(e));
                                        continue;
                                } catch (json::exception &err) {
                                        log_error(err, e);
                                }

                                log_error("Invalid event type", e);
                                continue;
                        }

                        switch (msg_type) {
                        case MsgType::Audio: {
                                try {
                                        container.emplace_back(
                                          events::RoomEvent<events::msg::Audio>(e));
                                } catch (json::exception &err) {
                                        log_error(err, e);
                                }

                                break;
                        }
                        case MsgType::Emote: {
                                try {
                                        container.emplace_back(
                                          events::RoomEvent<events::msg::Emote>(e));
                                } catch (json::exception &err) {
                                        log_error(err, e);
                                }

                                break;
                        }
                        case MsgType::File: {
                                try {
                                        container.emplace_back(
                                          events::RoomEvent<events::msg::File>(e));
                                } catch (json::exception &err) {
                                        log_error(err, e);
                                }

                                break;
                        }
                        case MsgType::Image: {
                                try {
                                        container.emplace_back(
                                          events::RoomEvent<events::msg::Image>(e));
                                } catch (json::exception &err) {
                                        log_error(err, e);
                                }

                                break;
                        }
                        case MsgType::Location: {
                                /* events::RoomEvent<events::msg::Location> location = e; */
                                /* container.emplace_back(location); */
                                break;
                        }
                        case MsgType::Notice: {
                                try {
                                        container.emplace_back(
                                          events::RoomEvent<events::msg::Notice>(e));
                                } catch (json::exception &err) {
                                        log_error(err, e);
                                }

                                break;
                        }
                        case MsgType::Text: {
                                try {
                                        container.emplace_back(
                                          events::RoomEvent<events::msg::Text>(e));
                                } catch (json::exception &err) {
                                        log_error(err, e);
                                }

                                break;
                        }
                        case MsgType::Video: {
                                try {
                                        container.emplace_back(
                                          events::RoomEvent<events::msg::Video>(e));
                                } catch (json::exception &err) {
                                        log_error(err, e);
                                }

                                break;
                        }
                        case MsgType::KeyVerificationRequest: {
                                try {
                                        container.emplace_back(
                                          events::RoomEvent<events::msg::KeyVerificationRequest>(
                                            e));
                                } catch (json::exception &err) {
                                        log_error(err, e);
                                }

                                break;
                        }
                        case MsgType::Unknown:
                                continue;
                        }
                        break;
                }
                case events::EventType::Sticker: {
                        try {
                                container.emplace_back(events::Sticker(e));
                        } catch (json::exception &err) {
                                log_error(err, e);
                        }

                        break;
                }
                case events::EventType::CallInvite: {
                        try {
                                container.emplace_back(
                                  events::RoomEvent<events::msg::CallInvite>(e));
                        } catch (json::exception &err) {
                                log_error(err, e);
                        }

                        break;
                }
                case events::EventType::CallCandidates: {
                        try {
                                container.emplace_back(
                                  events::RoomEvent<events::msg::CallCandidates>(e));
                        } catch (json::exception &err) {
                                log_error(err, e);
                        }

                        break;
                }
                case events::EventType::CallAnswer: {
                        try {
                                container.emplace_back(
                                  events::RoomEvent<events::msg::CallAnswer>(e));
                        } catch (json::exception &err) {
                                log_error(err, e);
                        }

                        break;
                }
                case events::EventType::CallHangUp: {
                        try {
                                container.emplace_back(
                                  events::RoomEvent<events::msg::CallHangUp>(e));
                        } catch (json::exception &err) {
                                log_error(err, e);
                        }

                        break;
                }
                case events::EventType::KeyVerificationRequest:
                case events::EventType::RoomPinnedEvents:
                case events::EventType::RoomKey:        // Not part of timeline or state
                case events::EventType::ForwardedRoomKey: // Not part of timeline or state
                case events::EventType::RoomKeyRequest: // Not part of the timeline
                case events::EventType::Tag:            // Not part of the timeline or state
                case events::EventType::Presence:       // Not part of the timeline or state
                case events::EventType::PushRules:      // Not part of the timeline or state
                case events::EventType::Unsupported:
                case events::EventType::NhekoHiddenEvents:
                        continue;
                }
        }
}

void
parse_device_events(const json &events,
                    std::vector<mtx::events::collections::DeviceEvents> &container)
{
        container.clear();
        container.reserve(events.size());
        for (const auto &e : events) {
                const auto type = mtx::events::getEventType(e);

                switch (type) {
                case events::EventType::RoomEncrypted: {
                        try {
                                const auto algo =
                                  e.at("content").at("algorithm").get<std::string>();
                                // Algorithm determines whether it's an olm or megolm event
                                if (algo == "m.olm.v1.curve25519-aes-sha2") {
                                        container.emplace_back(
                                          events::DeviceEvent<OlmEncrypted>(e));
                                } else if (algo == "m.megolm.v1.aes-sha2") {
                                        container.emplace_back(events::DeviceEvent<Encrypted>(e));
                                } else {
                                        log_error("Invalid m.room.encrypted algorithm", e);
                                        continue;
                                }
                        } catch (json::exception &err) {
                                log_error(err, e);
                        }

                        break;
                }
                case events::EventType::RoomKey: {
                        try {
                                container.emplace_back(events::DeviceEvent<RoomKey>(e));
                        } catch (json::exception &err) {
                                log_error(err, e);
                        }

                        break;
                }
                case events::EventType::ForwardedRoomKey: {
                        try {
                                container.emplace_back(events::DeviceEvent<ForwardedRoomKey>(e));
                        } catch (json::exception &err) {
                                log_error(err, e);
                        }

                        break;
                }
                case events::EventType::RoomKeyRequest: {
                        try {
                                container.emplace_back(events::DeviceEvent<KeyRequest>(e));
                        } catch (json::exception &err) {
                                log_error(err, e);
                        }

                        break;
                }
                case events::EventType::KeyVerificationCancel: {
                        try {
                                container.emplace_back(
                                  events::DeviceEvent<KeyVerificationCancel>(e));
                        } catch (json::exception &err) {
                                log_error(err, e);
                        }

                        break;
                }
                case events::EventType::KeyVerificationRequest:
                        try {
                                container.emplace_back(
                                  events::DeviceEvent<KeyVerificationRequest>(e));
                        } catch (json::exception &err) {
                                log_error(err, e);
                        }

                        break;
                case events::EventType::KeyVerificationStart:
                        try {
                                container.emplace_back(
                                  events::DeviceEvent<KeyVerificationStart>(e));
                        } catch (json::exception &err) {
                                log_error(err, e);
                        }

                        break;
                case events::EventType::KeyVerificationAccept:
                        try {
                                container.emplace_back(
                                  events::DeviceEvent<KeyVerificationAccept>(e));
                        } catch (json::exception &err) {
                                log_error(err, e);
                        }

                        break;
                case events::EventType::KeyVerificationKey:
                        try {
                                container.emplace_back(events::DeviceEvent<KeyVerificationKey>(e));
                        } catch (json::exception &err) {
                                log_error(err, e);
                        }

                        break;
                case events::EventType::KeyVerificationMac:
                        try {
                                container.emplace_back(events::DeviceEvent<KeyVerificationMac>(e));
                        } catch (json::exception &err) {
                                log_error(err, e);
                        }

                        break;
                case events::EventType::KeyVerificationReady:
                        try {
                                container.emplace_back(
                                  events::DeviceEvent<KeyVerificationReady>(e));
                        } catch (json::exception &err) {
                                log_error(err, e);
                        }

                        break;
                case events::EventType::KeyVerificationDone:
                        try {
                                container.emplace_back(events::DeviceEvent<KeyVerificationDone>(e));
                        } catch (json::exception &err) {
                                log_error(err, e);
                        }

                        break;
                default:
                        continue;
                }
        }
}

void
parse_state_events(const json &events,
                   std::vector<mtx::events::collections::StateEvents> &container)
{
        container.clear();
        container.reserve(events.size());

        for (const auto &e : events) {
                const auto type = mtx::events::getEventType(e);

                switch (type) {
                case events::EventType::RoomAliases: {
                        try {
                                container.emplace_back(events::StateEvent<Aliases>(e));
                        } catch (json::exception &err) {
                                log_error(err, e);
                        }

                        break;
                }
                case events::EventType::RoomAvatar: {
                        try {
                                container.emplace_back(events::StateEvent<Avatar>(e));
                        } catch (json::exception &err) {
                                log_error(err, e);
                        }

                        break;
                }
                case events::EventType::RoomCanonicalAlias: {
                        try {
                                container.emplace_back(events::StateEvent<CanonicalAlias>(e));
                        } catch (json::exception &err) {
                                log_error(err, e);
                        }

                        break;
                }
                case events::EventType::RoomCreate: {
                        try {
                                container.emplace_back(events::StateEvent<Create>(e));
                        } catch (json::exception &err) {
                                log_error(err, e);
                        }

                        break;
                }
                case events::EventType::RoomEncryption: {
                        try {
                                container.emplace_back(events::StateEvent<Encryption>(e));
                        } catch (json::exception &err) {
                                log_error(err, e);
                        }

                        break;
                }
                case events::EventType::RoomGuestAccess: {
                        try {
                                container.emplace_back(events::StateEvent<GuestAccess>(e));
                        } catch (json::exception &err) {
                                log_error(err, e);
                        }

                        break;
                }
                case events::EventType::RoomHistoryVisibility: {
                        try {
                                container.emplace_back(events::StateEvent<HistoryVisibility>(e));
                        } catch (json::exception &err) {
                                log_error(err, e);
                        }

                        break;
                }
                case events::EventType::RoomJoinRules: {
                        try {
                                container.emplace_back(events::StateEvent<JoinRules>(e));
                        } catch (json::exception &err) {
                                log_error(err, e);
                        }

                        break;
                }
                case events::EventType::RoomMember: {
                        try {
                                container.emplace_back(events::StateEvent<Member>(e));
                        } catch (json::exception &err) {
                                log_error(err, e);
                        }

                        break;
                }
                case events::EventType::RoomName: {
                        try {
                                container.emplace_back(events::StateEvent<Name>(e));
                        } catch (json::exception &err) {
                                log_error(err, e);
                        }

                        break;
                }
                case events::EventType::RoomPowerLevels: {
                        try {
                                container.emplace_back(events::StateEvent<PowerLevels>(e));
                        } catch (json::exception &err) {
                                log_error(err, e);
                        }

                        break;
                }
                case events::EventType::RoomTombstone: {
                        try {
                                container.emplace_back(events::StateEvent<Tombstone>(e));
                        } catch (json::exception &err) {
                                log_error(err, e);
                        }

                        break;
                }
                case events::EventType::RoomTopic: {
                        try {
                                container.emplace_back(events::StateEvent<Topic>(e));
                        } catch (json::exception &err) {
                                log_error(err, e);
                        }

                        break;
                }
                case events::EventType::Sticker:
                case events::EventType::Reaction:
                case events::EventType::RoomEncrypted:  /* Does this need to be here? */
                case events::EventType::RoomKey:        // Not part of timeline or state
                case events::EventType::ForwardedRoomKey: // Not part of timeline or state
                case events::EventType::RoomKeyRequest: // Not part of the timeline or state
                case events::EventType::RoomMessage:
                case events::EventType::RoomPinnedEvents:
                case events::EventType::RoomRedaction:
                case events::EventType::Tag:       // Not part of the timeline or state
                case events::EventType::Presence:  // Not part of the timeline or state
                case events::EventType::PushRules: // Not part of the timeline or state
                case events::EventType::Unsupported:
                case events::EventType::KeyVerificationCancel:
                case events::EventType::KeyVerificationRequest:
                case events::EventType::KeyVerificationStart:
                case events::EventType::KeyVerificationReady:
                case events::EventType::KeyVerificationDone:
                case events::EventType::KeyVerificationAccept:
                case events::EventType::KeyVerificationKey:
                case events::EventType::KeyVerificationMac:
                case events::EventType::CallInvite:
                case events::EventType::CallCandidates:
                case events::EventType::CallAnswer:
                case events::EventType::CallHangUp:
                case events::EventType::NhekoHiddenEvents:
                        continue;
                }
        }
}

void
parse_stripped_events(const json &events,
                      std::vector<mtx::events::collections::StrippedEvents> &container)
{
        container.clear();
        container.reserve(events.size());

        for (const auto &e : events) {
                const auto type = mtx::events::getEventType(e);

                switch (type) {
                case events::EventType::RoomAliases: {
                        try {
                                container.emplace_back(events::StrippedEvent<Aliases>(e));
                        } catch (json::exception &err) {
                                log_error(err, e);
                        }

                        break;
                }
                case events::EventType::RoomAvatar: {
                        try {
                                container.emplace_back(events::StrippedEvent<Avatar>(e));
                        } catch (json::exception &err) {
                                log_error(err, e);
                        }

                        break;
                }
                case events::EventType::RoomCanonicalAlias: {
                        try {
                                container.emplace_back(events::StrippedEvent<CanonicalAlias>(e));
                        } catch (json::exception &err) {
                                log_error(err, e);
                        }

                        break;
                }
                case events::EventType::RoomCreate: {
                        try {
                                container.emplace_back(events::StrippedEvent<Create>(e));
                        } catch (json::exception &err) {
                                log_error(err, e);
                        }

                        break;
                }
                case events::EventType::RoomGuestAccess: {
                        try {
                                container.emplace_back(events::StrippedEvent<GuestAccess>(e));
                        } catch (json::exception &err) {
                                log_error(err, e);
                        }

                        break;
                }
                case events::EventType::RoomHistoryVisibility: {
                        try {
                                container.emplace_back(events::StrippedEvent<HistoryVisibility>(e));
                        } catch (json::exception &err) {
                                log_error(err, e);
                        }

                        break;
                }
                case events::EventType::RoomJoinRules: {
                        try {
                                container.emplace_back(events::StrippedEvent<JoinRules>(e));
                        } catch (json::exception &err) {
                                log_error(err, e);
                        }

                        break;
                }
                case events::EventType::RoomMember: {
                        try {
                                container.emplace_back(events::StrippedEvent<Member>(e));
                        } catch (json::exception &err) {
                                log_error(err, e);
                        }

                        break;
                }
                case events::EventType::RoomName: {
                        try {
                                container.emplace_back(events::StrippedEvent<Name>(e));
                        } catch (json::exception &err) {
                                log_error(err, e);
                        }

                        break;
                }
                case events::EventType::RoomPowerLevels: {
                        try {
                                container.emplace_back(events::StrippedEvent<PowerLevels>(e));
                        } catch (json::exception &err) {
                                log_error(err, e);
                        }

                        break;
                }
                case events::EventType::RoomTombstone: {
                        try {
                                container.emplace_back(events::StrippedEvent<Tombstone>(e));
                        } catch (json::exception &err) {
                                log_error(err, e);
                        }

                        break;
                }
                case events::EventType::RoomTopic: {
                        try {
                                container.emplace_back(events::StrippedEvent<Topic>(e));
                        } catch (json::exception &err) {
                                log_error(err, e);
                        }

                        break;
                }
                case events::EventType::Sticker:
                case events::EventType::Reaction:
                case events::EventType::RoomEncrypted:
                case events::EventType::RoomEncryption:
                case events::EventType::RoomMessage:
                case events::EventType::RoomRedaction:
                case events::EventType::RoomKey:        // Not part of timeline or state
                case events::EventType::ForwardedRoomKey: // Not part of timeline or state
                case events::EventType::RoomKeyRequest: // Not part of the timeline or state
                case events::EventType::RoomPinnedEvents:
                case events::EventType::Tag:       // Not part of the timeline or state
                case events::EventType::Presence:  // Not part of the timeline or state
                case events::EventType::PushRules: // Not part of the timeline or state
                case events::EventType::Unsupported:
                case events::EventType::KeyVerificationCancel:
                case events::EventType::KeyVerificationRequest:
                case events::EventType::KeyVerificationStart:
                case events::EventType::KeyVerificationReady:
                case events::EventType::KeyVerificationDone:
                case events::EventType::KeyVerificationAccept:
                case events::EventType::KeyVerificationKey:
                case events::EventType::KeyVerificationMac:
                case events::EventType::CallInvite:
                case events::EventType::CallCandidates:
                case events::EventType::CallAnswer:
                case events::EventType::CallHangUp:
                case events::EventType::NhekoHiddenEvents:
                        continue;
                }
        }
}
}
}
}
