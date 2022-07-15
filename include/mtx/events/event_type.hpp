#pragma once

/// @file
/// @brief Enumeration of all event types

#if __has_include(<nlohmann/json_fwd.hpp>)
#include <nlohmann/json_fwd.hpp>
#else
#include <nlohmann/json.hpp>
#endif

#include <string>
namespace mtx {
namespace events {

//! The type of an event.
enum class EventType
{
    /// m.key.verification.cancel
    KeyVerificationCancel,
    /// m.key.verification.request
    KeyVerificationRequest,
    /// m.key.verification.start
    KeyVerificationStart,
    /// m.key.verification.accept
    KeyVerificationAccept,
    /// m.key.verification.key
    KeyVerificationKey,
    /// m.key.verification.mac
    KeyVerificationMac,
    /// m.key.verification.ready,
    KeyVerificationReady,
    /// m.key.verification.done,
    KeyVerificationDone,
    /// m.reaction,
    Reaction,
    /// m.room_key
    RoomKey,
    /// m.forwarded_room_key
    ForwardedRoomKey,
    /// m.room_key_request
    RoomKeyRequest,
    /// m.room.aliases
    RoomAliases,
    /// m.room.avatar
    RoomAvatar,
    /// m.room.canonical_alias
    RoomCanonicalAlias,
    /// m.room.create
    RoomCreate,
    /// m.room.encrypted.
    RoomEncrypted,
    /// m.room.encryption.
    RoomEncryption,
    /// m.room.guest_access
    RoomGuestAccess,
    /// m.room.history_visibility
    RoomHistoryVisibility,
    /// m.room.join_rules
    RoomJoinRules,
    /// m.room.member
    RoomMember,
    /// m.room.message
    RoomMessage,
    /// m.room.name
    RoomName,
    /// m.room.power_levels
    RoomPowerLevels,
    /// m.room.topic
    RoomTopic,
    /// m.room.redaction
    RoomRedaction,
    /// m.room.pinned_events
    RoomPinnedEvents,
    /// m.room.tombstone
    RoomTombstone,
    // m.sticker
    Sticker,
    // m.tag
    Tag,
    // m.presence
    Presence,
    // m.push_rules
    PushRules,

    //! m.widget
    Widget,
    //! im.vector.modular.widgets
    VectorWidget,

    //! m.policy.rule.user
    PolicyRuleUser,
    //! m.policy.rule.room
    PolicyRuleRoom,
    //! m.policy.rule.server
    PolicyRuleServer,

    // m.space.child
    SpaceChild,
    // m.space.parent
    SpaceParent,

    // m.call.invite
    CallInvite,
    // m.call.candidates
    CallCandidates,
    // m.call.answer
    CallAnswer,
    // m.call.hangup
    CallHangUp,
    // m.call.select_answer
    CallSelectAnswer,
    // m.call.reject
    CallReject,
    // m.call.negotiate
    CallNegotiate,

    // m.secret.request
    SecretRequest,
    // m.secret.send
    SecretSend,

    //! m.typing
    Typing,
    //! m.receipt
    Receipt,
    //! m.fully_read
    FullyRead,
    //! m.direct
    Direct,

    // custom events
    // im.nheko.hidden_events
    NhekoHiddenEvents,

    // MSCs
    //! m.image_pack, currently im.ponies.room_emotes
    ImagePackInRoom,
    //! m.image_pack, currently im.ponies.user_emotes
    ImagePackInAccountData,
    //! m.image_pack.rooms, currently im.ponies.emote_rooms
    ImagePackRooms,

    //! `m.dummy`, used in crypto for example
    Dummy,

    //! Unsupported event
    Unsupported,
};

//! Turn an event into a string
std::string
to_string(EventType type);

//! Parse a string into an event type.
EventType
getEventType(const std::string &type);

//! Get the event type of an event.
EventType
getEventType(const nlohmann::json &obj);
}
}
