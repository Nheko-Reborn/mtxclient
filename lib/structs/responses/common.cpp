#include "mtx/responses/common.hpp"

#include <nlohmann/json.hpp>

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
#include "mtx/log.hpp"

using json = nlohmann::json;
using namespace mtx::events::account_data;
using namespace mtx::events::state;
using namespace mtx::events::msg;

namespace mtx {
namespace responses {
void
from_json(const nlohmann::json &obj, RoomId &response)
{
    response.room_id = obj.at("room_id").get<std::string>();
}

void
from_json(const nlohmann::json &obj, EventId &response)
{
    response.event_id = obj.at("event_id").get<mtx::identifiers::Event>();
}

void
from_json(const nlohmann::json &obj, FilterId &response)
{
    response.filter_id = obj.at("filter_id").get<std::string>();
}

void
from_json(const nlohmann::json &obj, Version &response)
{
    response.version = obj.at("version").get<std::string>();
}

void
from_json(const nlohmann::json &obj, Success &success)
{
    success.success = obj.at("success").get<bool>();
}

void
from_json(const nlohmann::json &obj, Available &response)
{
    response.available = obj.at("available").get<bool>();
}

void
from_json(const nlohmann::json &obj, RequestToken &r)
{
    r.sid = obj.at("sid").get<std::string>();

    if (obj.contains("submit_url"))
        r.submit_url = obj.at("submit_url").get<std::string>();
}

void
from_json(const nlohmann::json &obj, Aliases &response)
{
    response.aliases = obj.at("aliases").get<std::vector<std::string>>();
}

namespace utils {

void
log_error(std::exception &err, const json &event)
{
    mtx::utils::log::log()->warn("Error parsing events: {}, {}", err.what(), event.dump(2));
}

void
log_error(const std::string &err, const json &event)
{
    mtx::utils::log::log()->warn("Error parsing events: {}, {}", err, event.dump(2));
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

        // if (!e.contains("content") || e["content"].empty()) {
        //        container.emplace_back(events::AccountDataEvent<events::msg::Redacted>(e));
        //        break;
        //}

        try {
            switch (type) {
            case events::EventType::Direct: {
                container.emplace_back(events::AccountDataEvent<Direct>(e));
                break;
            }
            case events::EventType::Tag: {
                container.emplace_back(events::AccountDataEvent<Tags>(e));
                break;
            }
            case events::EventType::FullyRead: {
                container.emplace_back(
                  events::AccountDataEvent<events::account_data::FullyRead>(e));
                break;
            }
            case events::EventType::PushRules: {
                container.emplace_back(events::AccountDataEvent<pushrules::GlobalRuleset>(e));
                break;
            }
            case events::EventType::NhekoHiddenEvents: {
                container.emplace_back(events::AccountDataEvent<nheko_extensions::HiddenEvents>(e));
                break;
            }
            case events::EventType::ImagePackRooms: {
                container.emplace_back(
                  events::AccountDataEvent<events::msc2545::ImagePackRooms>(e));
                break;
            }
            case events::EventType::ImagePackInAccountData: {
                container.emplace_back(events::AccountDataEvent<events::msc2545::ImagePack>(e));
                break;
            }
            case events::EventType::Unsupported: {
                container.emplace_back(events::EphemeralEvent<events::Unknown>(e));
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
            case events::EventType::SecretRequest:
            case events::EventType::SecretSend:
            case events::EventType::Presence:
            case events::EventType::Reaction:
            case events::EventType::RoomAliases:
            case events::EventType::RoomAvatar:
            case events::EventType::RoomCanonicalAlias:
            case events::EventType::RoomCreate:
            case events::EventType::RoomEncrypted:
            case events::EventType::Dummy:
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
            case events::EventType::Widget:
            case events::EventType::VectorWidget:
            case events::EventType::PolicyRuleUser:
            case events::EventType::PolicyRuleRoom:
            case events::EventType::PolicyRuleServer:
            case events::EventType::SpaceChild:
            case events::EventType::SpaceParent:
            case events::EventType::Sticker:
            case events::EventType::CallInvite:
            case events::EventType::CallCandidates:
            case events::EventType::CallAnswer:
            case events::EventType::CallHangUp:
            case events::EventType::CallSelectAnswer:
            case events::EventType::CallReject:
            case events::EventType::CallNegotiate:
            case events::EventType::Typing:
            case events::EventType::Receipt:
            case events::EventType::ImagePackInRoom:
                continue;
            }
        } catch (std::exception &err) {
            log_error(err, e);
        }
    }
}

void
compose_timeline_events(json &events,
                        const std::vector<mtx::events::collections::TimelineEvents> &container)
{
    const auto &c = container.at(0);
    events        = std::visit([](auto e) { return json(e); }, c);
}

void
parse_timeline_events(const json &events,
                      std::vector<mtx::events::collections::TimelineEvents> &container)
{
    container.clear();
    container.reserve(events.size());

    for (const auto &e : events) {
        const auto type = mtx::events::getEventType(e);

        try {
            if (e.contains("unsigned") && e["unsigned"].contains("redacted_by")) {
                if (e.contains("state_key"))
                    container.emplace_back(events::StateEvent<events::msg::Redacted>(e));
                else
                    container.emplace_back(events::RoomEvent<events::msg::Redacted>(e));
                continue;
            }
            switch (type) {
            case events::EventType::Reaction: {
                container.emplace_back(events::RoomEvent<events::msg::Reaction>(e));
                break;
            }
            case events::EventType::RoomAliases: {
                container.emplace_back(events::StateEvent<events::state::Aliases>(e));
                break;
            }
            case events::EventType::RoomAvatar: {
                container.emplace_back(events::StateEvent<Avatar>(e));
                break;
            }
            case events::EventType::RoomCanonicalAlias: {
                container.emplace_back(events::StateEvent<CanonicalAlias>(e));
                break;
            }
            case events::EventType::RoomCreate: {
                container.emplace_back(events::StateEvent<Create>(e));
                break;
            }
            case events::EventType::RoomEncrypted: {
                container.emplace_back(events::EncryptedEvent<mtx::events::msg::Encrypted>(e));
                break;
            }
            case events::EventType::RoomEncryption: {
                container.emplace_back(events::StateEvent<Encryption>(e));
                break;
            }
            case events::EventType::RoomGuestAccess: {
                container.emplace_back(events::StateEvent<GuestAccess>(e));

                break;
            }
            case events::EventType::RoomHistoryVisibility: {
                container.emplace_back(events::StateEvent<HistoryVisibility>(e));

                break;
            }
            case events::EventType::RoomJoinRules: {
                container.emplace_back(events::StateEvent<JoinRules>(e));

                break;
            }
            case events::EventType::RoomMember: {
                container.emplace_back(events::StateEvent<Member>(e));

                break;
            }
            case events::EventType::RoomName: {
                container.emplace_back(events::StateEvent<Name>(e));

                break;
            }
            case events::EventType::RoomPowerLevels: {
                container.emplace_back(events::StateEvent<PowerLevels>(e));

                break;
            }
            case events::EventType::RoomRedaction: {
                container.emplace_back(events::RedactionEvent<mtx::events::msg::Redaction>(e));

                break;
            }
            case events::EventType::RoomTombstone: {
                container.emplace_back(events::StateEvent<Tombstone>(e));

                break;
            }
            case events::EventType::RoomTopic: {
                container.emplace_back(events::StateEvent<Topic>(e));

                break;
            }
            case events::EventType::Widget: {
                container.emplace_back(events::StateEvent<Widget>(e));

                break;
            }
            case events::EventType::VectorWidget: {
                container.emplace_back(events::StateEvent<Widget>(e));

                break;
            }
            case events::EventType::PolicyRuleUser: {
                container.emplace_back(events::StateEvent<policy_rule::UserRule>(e));

                break;
            }
            case events::EventType::PolicyRuleRoom: {
                container.emplace_back(events::StateEvent<policy_rule::RoomRule>(e));

                break;
            }
            case events::EventType::PolicyRuleServer: {
                container.emplace_back(events::StateEvent<policy_rule::ServerRule>(e));

                break;
            }
            case events::EventType::SpaceChild: {
                container.emplace_back(events::StateEvent<space::Child>(e));

                break;
            }
            case events::EventType::SpaceParent: {
                container.emplace_back(events::StateEvent<space::Parent>(e));

                break;
            }
            case events::EventType::ImagePackInRoom: {
                container.emplace_back(events::StateEvent<events::msc2545::ImagePack>(e));

                break;
            }
            case events::EventType::KeyVerificationStart: {
                container.emplace_back(events::RoomEvent<events::msg::KeyVerificationStart>(e));

                break;
            }
            case events::EventType::KeyVerificationAccept: {
                container.emplace_back(events::RoomEvent<events::msg::KeyVerificationAccept>(e));

                break;
            }
            case events::EventType::KeyVerificationDone: {
                container.emplace_back(events::RoomEvent<events::msg::KeyVerificationDone>(e));

                break;
            }
            case events::EventType::KeyVerificationReady: {
                container.emplace_back(events::RoomEvent<events::msg::KeyVerificationReady>(e));

                break;
            }
            case events::EventType::KeyVerificationKey: {
                container.emplace_back(events::RoomEvent<events::msg::KeyVerificationKey>(e));

                break;
            }
            case events::EventType::KeyVerificationMac: {
                container.emplace_back(events::RoomEvent<events::msg::KeyVerificationMac>(e));

                break;
            }
            case events::EventType::KeyVerificationCancel: {
                container.emplace_back(events::RoomEvent<events::msg::KeyVerificationCancel>(e));

                break;
            }
            case events::EventType::RoomMessage: {
                using MsgType       = mtx::events::MessageType;
                const auto msg_type = mtx::events::getMessageType(e.at("content"));

                switch (msg_type) {
                case MsgType::Audio: {
                    container.emplace_back(events::RoomEvent<events::msg::Audio>(e));

                    break;
                }
                case MsgType::Confetti: {
                    container.emplace_back(events::RoomEvent<events::msg::Confetti>(e));

                    break;
                }
                case MsgType::Emote: {
                    container.emplace_back(events::RoomEvent<events::msg::Emote>(e));

                    break;
                }
                case MsgType::File: {
                    container.emplace_back(events::RoomEvent<events::msg::File>(e));

                    break;
                }
                case MsgType::Image: {
                    container.emplace_back(events::RoomEvent<events::msg::Image>(e));

                    break;
                }
                case MsgType::Location: {
                    /* events::RoomEvent<events::msg::Location> location = e; */
                    /* container.emplace_back(location); */
                    break;
                }
                case MsgType::Notice: {
                    container.emplace_back(events::RoomEvent<events::msg::Notice>(e));

                    break;
                }
                case MsgType::Text: {
                    container.emplace_back(events::RoomEvent<events::msg::Text>(e));

                    break;
                }
                case MsgType::Video: {
                    container.emplace_back(events::RoomEvent<events::msg::Video>(e));

                    break;
                }
                case MsgType::KeyVerificationRequest: {
                    container.emplace_back(
                      events::RoomEvent<events::msg::KeyVerificationRequest>(e));

                    break;
                }
                case MsgType::Unknown: {
                    container.emplace_back(events::RoomEvent<events::Unknown>(e));
                    break;
                }
                }
                break;
            }
            case events::EventType::Sticker: {
                container.emplace_back(events::Sticker(e));

                break;
            }
            case events::EventType::CallInvite: {
                container.emplace_back(events::RoomEvent<events::voip::CallInvite>(e));

                break;
            }
            case events::EventType::CallCandidates: {
                container.emplace_back(events::RoomEvent<events::voip::CallCandidates>(e));

                break;
            }
            case events::EventType::CallAnswer: {
                container.emplace_back(events::RoomEvent<events::voip::CallAnswer>(e));

                break;
            }
            case events::EventType::CallHangUp: {
                container.emplace_back(events::RoomEvent<events::voip::CallHangUp>(e));

                break;
            }
            case events::EventType::CallSelectAnswer: {
                container.emplace_back(events::RoomEvent<events::voip::CallSelectAnswer>(e));

                break;
            }
            case events::EventType::CallReject: {
                container.emplace_back(events::RoomEvent<events::voip::CallReject>(e));

                break;
            }
            case events::EventType::CallNegotiate: {
                container.emplace_back(events::RoomEvent<events::voip::CallNegotiate>(e));

                break;
            }
            case events::EventType::RoomPinnedEvents: {
                container.emplace_back(events::StateEvent<events::state::PinnedEvents>(e));

                break;
            }
            case events::EventType::Unsupported: {
                container.emplace_back(events::RoomEvent<events::Unknown>(e));

                break;
            }
            case events::EventType::KeyVerificationRequest:
            case events::EventType::RoomKey:          // Not part of timeline or state
            case events::EventType::ForwardedRoomKey: // Not part of timeline or state
            case events::EventType::RoomKeyRequest:   // Not part of the timeline
            case events::EventType::Direct:           // Not part of the timeline or state
            case events::EventType::Tag:              // Not part of the timeline or state
            case events::EventType::Presence:         // Not part of the timeline or state
            case events::EventType::PushRules:        // Not part of the timeline or state
            case events::EventType::SecretRequest:    // Not part of the timeline or state
            case events::EventType::SecretSend:       // Not part of the timeline or state
            case events::EventType::Typing:
            case events::EventType::Receipt:
            case events::EventType::FullyRead:
            case events::EventType::NhekoHiddenEvents:
            case events::EventType::ImagePackRooms:
            case events::EventType::ImagePackInAccountData:
            case events::EventType::Dummy:
                continue;
            }
        } catch (std::exception &err) {
            log_error(err, e);
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

        try {
            switch (type) {
            case events::EventType::RoomEncrypted: {
                const auto algo = e.at("content").at("algorithm").get<std::string>();
                // Algorithm determines whether it's an olm or megolm event
                if (algo == "m.olm.v1.curve25519-aes-sha2") {
                    container.emplace_back(events::DeviceEvent<OlmEncrypted>(e));
                } else if (algo == "m.megolm.v1.aes-sha2") {
                    container.emplace_back(events::DeviceEvent<Encrypted>(e));
                } else {
                    log_error("Invalid m.room.encrypted algorithm", e);
                    continue;
                }
                break;
            }
            case events::EventType::Dummy: {
                container.emplace_back(events::DeviceEvent<Dummy>(e));

                break;
            }
            case events::EventType::RoomKey: {
                container.emplace_back(events::DeviceEvent<RoomKey>(e));

                break;
            }
            case events::EventType::ForwardedRoomKey: {
                container.emplace_back(events::DeviceEvent<ForwardedRoomKey>(e));

                break;
            }
            case events::EventType::RoomKeyRequest: {
                container.emplace_back(events::DeviceEvent<KeyRequest>(e));

                break;
            }
            case events::EventType::KeyVerificationCancel: {
                container.emplace_back(events::DeviceEvent<KeyVerificationCancel>(e));

                break;
            }
            case events::EventType::KeyVerificationRequest:
                container.emplace_back(events::DeviceEvent<KeyVerificationRequest>(e));

                break;
            case events::EventType::KeyVerificationStart:
                container.emplace_back(events::DeviceEvent<KeyVerificationStart>(e));

                break;
            case events::EventType::KeyVerificationAccept:
                container.emplace_back(events::DeviceEvent<KeyVerificationAccept>(e));

                break;
            case events::EventType::KeyVerificationKey:
                container.emplace_back(events::DeviceEvent<KeyVerificationKey>(e));

                break;
            case events::EventType::KeyVerificationMac:
                container.emplace_back(events::DeviceEvent<KeyVerificationMac>(e));

                break;
            case events::EventType::KeyVerificationReady:
                container.emplace_back(events::DeviceEvent<KeyVerificationReady>(e));

                break;
            case events::EventType::KeyVerificationDone:
                container.emplace_back(events::DeviceEvent<KeyVerificationDone>(e));

                break;
            case events::EventType::SecretSend:
                container.emplace_back(events::DeviceEvent<SecretSend>(e));

                break;
            case events::EventType::SecretRequest:
                container.emplace_back(events::DeviceEvent<SecretRequest>(e));

                break;
            case events::EventType::Unsupported:
                container.emplace_back(events::DeviceEvent<events::Unknown>(e));

                break;
            default:
                continue;
            }
        } catch (std::exception &err) {
            log_error(err, e);
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

        try {
            if (e.contains("unsigned") && e["unsigned"].contains("redacted_by")) {
                container.emplace_back(events::StateEvent<events::msg::Redacted>(e));
                continue;
            }

            switch (type) {
            case events::EventType::RoomAliases: {
                container.emplace_back(events::StateEvent<events::state::Aliases>(e));

                break;
            }
            case events::EventType::RoomAvatar: {
                container.emplace_back(events::StateEvent<Avatar>(e));
                break;
            }
            case events::EventType::RoomCanonicalAlias: {
                container.emplace_back(events::StateEvent<CanonicalAlias>(e));

                break;
            }
            case events::EventType::RoomCreate: {
                container.emplace_back(events::StateEvent<Create>(e));

                break;
            }
            case events::EventType::RoomEncryption: {
                container.emplace_back(events::StateEvent<Encryption>(e));

                break;
            }
            case events::EventType::RoomGuestAccess: {
                container.emplace_back(events::StateEvent<GuestAccess>(e));

                break;
            }
            case events::EventType::RoomHistoryVisibility: {
                container.emplace_back(events::StateEvent<HistoryVisibility>(e));

                break;
            }
            case events::EventType::RoomJoinRules: {
                container.emplace_back(events::StateEvent<JoinRules>(e));

                break;
            }
            case events::EventType::RoomMember: {
                container.emplace_back(events::StateEvent<Member>(e));

                break;
            }
            case events::EventType::RoomName: {
                container.emplace_back(events::StateEvent<Name>(e));

                break;
            }
            case events::EventType::RoomPowerLevels: {
                container.emplace_back(events::StateEvent<PowerLevels>(e));

                break;
            }
            case events::EventType::RoomTombstone: {
                container.emplace_back(events::StateEvent<Tombstone>(e));

                break;
            }
            case events::EventType::RoomTopic: {
                container.emplace_back(events::StateEvent<Topic>(e));

                break;
            }
            case events::EventType::Widget: {
                container.emplace_back(events::StateEvent<Widget>(e));

                break;
            }
            case events::EventType::VectorWidget: {
                container.emplace_back(events::StateEvent<Widget>(e));

                break;
            }
            case events::EventType::PolicyRuleUser: {
                container.emplace_back(events::StateEvent<policy_rule::UserRule>(e));

                break;
            }
            case events::EventType::PolicyRuleRoom: {
                container.emplace_back(events::StateEvent<policy_rule::RoomRule>(e));

                break;
            }
            case events::EventType::PolicyRuleServer: {
                container.emplace_back(events::StateEvent<policy_rule::ServerRule>(e));

                break;
            }
            case events::EventType::SpaceChild: {
                container.emplace_back(events::StateEvent<space::Child>(e));

                break;
            }
            case events::EventType::SpaceParent: {
                container.emplace_back(events::StateEvent<space::Parent>(e));

                break;
            }
            case events::EventType::ImagePackInRoom: {
                container.emplace_back(events::StateEvent<events::msc2545::ImagePack>(e));

                break;
            }
            case events::EventType::RoomPinnedEvents: {
                container.emplace_back(events::StateEvent<events::state::PinnedEvents>(e));

                break;
            }
            case events::EventType::Unsupported: {
                container.emplace_back(events::StateEvent<events::Unknown>(e));

                break;
            }
            case events::EventType::Sticker:
            case events::EventType::Reaction:
            case events::EventType::RoomEncrypted:    /* Does this need to be here? */
            case events::EventType::RoomKey:          // Not part of timeline or state
            case events::EventType::ForwardedRoomKey: // Not part of timeline or state
            case events::EventType::RoomKeyRequest:   // Not part of the timeline or state
            case events::EventType::RoomMessage:
            case events::EventType::RoomRedaction:
            case events::EventType::Direct:    // Not part of the timeline or state
            case events::EventType::Tag:       // Not part of the timeline or state
            case events::EventType::Presence:  // Not part of the timeline or state
            case events::EventType::PushRules: // Not part of the timeline or state
            case events::EventType::KeyVerificationCancel:
            case events::EventType::KeyVerificationRequest:
            case events::EventType::KeyVerificationStart:
            case events::EventType::KeyVerificationReady:
            case events::EventType::KeyVerificationDone:
            case events::EventType::KeyVerificationAccept:
            case events::EventType::KeyVerificationKey:
            case events::EventType::KeyVerificationMac:
            case events::EventType::SecretRequest:
            case events::EventType::SecretSend:
            case events::EventType::CallInvite:
            case events::EventType::CallCandidates:
            case events::EventType::CallAnswer:
            case events::EventType::CallHangUp:
            case events::EventType::CallSelectAnswer:
            case events::EventType::CallReject:
            case events::EventType::CallNegotiate:
            case events::EventType::Typing:
            case events::EventType::Receipt:
            case events::EventType::FullyRead:
            case events::EventType::NhekoHiddenEvents:
            case events::EventType::ImagePackRooms:
            case events::EventType::ImagePackInAccountData:
            case events::EventType::Dummy:
                continue;
            }
        } catch (std::exception &err) {
            log_error(err, e);
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

        try {
            switch (type) {
            case events::EventType::RoomAliases: {
                container.emplace_back(events::StrippedEvent<mtx::events::state::Aliases>(e));

                break;
            }
            case events::EventType::RoomAvatar: {
                container.emplace_back(events::StrippedEvent<Avatar>(e));

                break;
            }
            case events::EventType::RoomCanonicalAlias: {
                container.emplace_back(events::StrippedEvent<CanonicalAlias>(e));

                break;
            }
            case events::EventType::RoomCreate: {
                container.emplace_back(events::StrippedEvent<Create>(e));

                break;
            }
            case events::EventType::RoomGuestAccess: {
                container.emplace_back(events::StrippedEvent<GuestAccess>(e));

                break;
            }
            case events::EventType::RoomHistoryVisibility: {
                container.emplace_back(events::StrippedEvent<HistoryVisibility>(e));

                break;
            }
            case events::EventType::RoomJoinRules: {
                container.emplace_back(events::StrippedEvent<JoinRules>(e));

                break;
            }
            case events::EventType::RoomMember: {
                container.emplace_back(events::StrippedEvent<Member>(e));

                break;
            }
            case events::EventType::RoomName: {
                container.emplace_back(events::StrippedEvent<Name>(e));

                break;
            }
            case events::EventType::RoomPowerLevels: {
                container.emplace_back(events::StrippedEvent<PowerLevels>(e));

                break;
            }
            case events::EventType::RoomTombstone: {
                container.emplace_back(events::StrippedEvent<Tombstone>(e));

                break;
            }
            case events::EventType::RoomTopic: {
                container.emplace_back(events::StrippedEvent<Topic>(e));

                break;
            }
            case events::EventType::Widget: {
                container.emplace_back(events::StrippedEvent<Widget>(e));

                break;
            }
            case events::EventType::VectorWidget: {
                container.emplace_back(events::StrippedEvent<Widget>(e));

                break;
            }
            case events::EventType::PolicyRuleUser: {
                container.emplace_back(events::StrippedEvent<policy_rule::UserRule>(e));

                break;
            }
            case events::EventType::PolicyRuleRoom: {
                container.emplace_back(events::StrippedEvent<policy_rule::RoomRule>(e));

                break;
            }
            case events::EventType::PolicyRuleServer: {
                container.emplace_back(events::StrippedEvent<policy_rule::ServerRule>(e));

                break;
            }
            case events::EventType::SpaceChild: {
                container.emplace_back(events::StrippedEvent<space::Child>(e));

                break;
            }
            case events::EventType::SpaceParent: {
                container.emplace_back(events::StrippedEvent<space::Parent>(e));

                break;
            }
            case events::EventType::RoomPinnedEvents: {
                container.emplace_back(events::StrippedEvent<events::state::PinnedEvents>(e));

                break;
            }
            case events::EventType::Unsupported: {
                container.emplace_back(events::StrippedEvent<events::Unknown>(e));

                break;
            }
            case events::EventType::Sticker:
            case events::EventType::Reaction:
            case events::EventType::RoomEncrypted:
            case events::EventType::RoomEncryption:
            case events::EventType::RoomMessage:
            case events::EventType::RoomRedaction:
            case events::EventType::RoomKey:          // Not part of timeline or state
            case events::EventType::ForwardedRoomKey: // Not part of timeline or state
            case events::EventType::RoomKeyRequest:   // Not part of the timeline or state
            case events::EventType::Direct:           // Not part of the timeline or state
            case events::EventType::Tag:              // Not part of the timeline or state
            case events::EventType::Presence:         // Not part of the timeline or state
            case events::EventType::PushRules:        // Not part of the timeline or state
            case events::EventType::KeyVerificationCancel:
            case events::EventType::KeyVerificationRequest:
            case events::EventType::KeyVerificationStart:
            case events::EventType::KeyVerificationReady:
            case events::EventType::KeyVerificationDone:
            case events::EventType::KeyVerificationAccept:
            case events::EventType::KeyVerificationKey:
            case events::EventType::KeyVerificationMac:
            case events::EventType::SecretRequest:
            case events::EventType::SecretSend:
            case events::EventType::CallInvite:
            case events::EventType::CallCandidates:
            case events::EventType::CallAnswer:
            case events::EventType::CallHangUp:
            case events::EventType::CallSelectAnswer:
            case events::EventType::CallReject:
            case events::EventType::CallNegotiate:
            case events::EventType::Typing:
            case events::EventType::Receipt:
            case events::EventType::FullyRead:
            case events::EventType::NhekoHiddenEvents:
            case events::EventType::ImagePackInAccountData:
            case events::EventType::ImagePackInRoom:
            case events::EventType::ImagePackRooms:
            case events::EventType::Dummy:
                continue;
            }
        } catch (std::exception &err) {
            log_error(err, e);
        }
    }
}

void
parse_ephemeral_events(const json &events,
                       std::vector<mtx::events::collections::EphemeralEvents> &container)
{
    container.clear();
    container.reserve(events.size());
    for (const auto &e : events) {
        const auto type = mtx::events::getEventType(e);

        try {
            switch (type) {
            case events::EventType::Typing: {
                container.emplace_back(events::EphemeralEvent<events::ephemeral::Typing>(e));
                break;
            }
            case events::EventType::Receipt: {
                container.emplace_back(events::EphemeralEvent<events::ephemeral::Receipt>(e));

                break;
            }
            case events::EventType::Unsupported: {
                container.emplace_back(events::EphemeralEvent<events::Unknown>(e));

                break;
            }
            default:
                continue;
            }
        } catch (std::exception &err) {
            utils::log_error(err, e);
        }
    }
}
}

void
from_json(const nlohmann::json &arr, StateEvents &response)
{
    utils::parse_state_events(arr, response.events);
}
}
}
