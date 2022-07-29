#pragma once

/// @file
/// @brief Call related events.

#if __has_include(<nlohmann/json_fwd.hpp>)
#include <nlohmann/json_fwd.hpp>
#else
#include <nlohmann/json.hpp>
#endif

#include <string>
#include <vector>

namespace mtx {
namespace events {
namespace voip {

//! Universal RTC Session Description structure compatible with the WebRTC API
struct RTCSessionDescriptionInit
{
    enum class Type
    {
        Answer,
        Offer,

    };
    //! The SDP text of the session description.
    std::string sdp;
    //! The type of session description.
    Type type;

    friend void from_json(const nlohmann::json &obj, RTCSessionDescriptionInit &content);
    friend void to_json(nlohmann::json &obj, const RTCSessionDescriptionInit &content);
};

//! Content for the `m.call.invite` event.
struct CallInvite
{
    //! A unique identifier for the call.
    std::string call_id;
    //! A unique identifier for the client participating in the event.
    std::string party_id;
    //! The session description object
    RTCSessionDescriptionInit offer;
    //! The version of the VoIP specification this message adheres to.
    std::string version;
    //! The time in milliseconds that the invite is valid for. Recommended 90000ms
    uint32_t lifetime = 90000;
    //! The ID of the user that the call is being placed to. See MSC2746.
    std::string invitee;

    friend void from_json(const nlohmann::json &obj, CallInvite &content);
    friend void to_json(nlohmann::json &obj, const CallInvite &content);
};

//! Content for the `m.call.candidates` event
struct CallCandidates
{
    struct Candidate
    {
        //! The SDP media type this candidate is intended for.
        std::string sdpMid;
        //! The index of the SDP 'm' line this candidate is intended for.
        uint16_t sdpMLineIndex;
        //! The SDP 'a' line of the candidate.
        std::string candidate;
    };

    //! The ID of the call this event relates to.
    std::string call_id;
    //! A unique identifier for the client participating in the event.
    std::string party_id;
    //! Array of objects describing the candidates.
    std::vector<Candidate> candidates;
    //! The version of the VoIP specification this message adheres to.
    std::string version;

    friend void from_json(const nlohmann::json &obj, CallCandidates &content);
    friend void to_json(nlohmann::json &obj, const CallCandidates &content);
};

//! Content for the `m.call.answer` event
struct CallAnswer
{
    //! The ID of the call this event relates to.
    std::string call_id;
    //! A unique identifier for the client participating in the event.
    std::string party_id;
    //! The version of the VoIP specification this message adheres to.
    std::string version;
    //! The session description object
    RTCSessionDescriptionInit answer;

    friend void from_json(const nlohmann::json &obj, CallAnswer &content);
    friend void to_json(nlohmann::json &obj, const CallAnswer &content);
};

//! Content for the `m.call.hangup` event
struct CallHangUp
{
    enum class Reason
    {
        ICEFailed,
        InviteTimeOut,
        ICETimeOut,
        UserHangUp,
        UserMediaFailed,
        UserBusy,
        UnknownError,
        User
    };

    //! The ID of the call this event relates to.
    std::string call_id;
    //! A unique identifier for the client participating in the event.
    std::string party_id;
    //! The version of the VoIP specification this message adheres to.
    std::string version;
    //! The reason for the call hang up.
    Reason reason = Reason::UserHangUp;

    friend void from_json(const nlohmann::json &obj, CallHangUp &content);
    friend void to_json(nlohmann::json &obj, const CallHangUp &content);
};

//! Content for the `m.call.select_answer` event. See MSC2746.
struct CallSelectAnswer
{
    //! The ID of the call this event relates to.
    std::string call_id;
    //! A unique identifier for the client participating in the event.
    std::string party_id;
    //! The version of the VoIP specification this message adheres to.
    std::string version;
    //! The ID of the selected party.
    std::string selected_party_id;

    friend void from_json(const nlohmann::json &obj, CallSelectAnswer &content);
    friend void to_json(nlohmann::json &obj, const CallSelectAnswer &content);
};

//! Content for the `m.call.reject` event. See MSC2746.
struct CallReject
{
    //! The ID of the call this event relates to.
    std::string call_id;
    //! A unique identifier for the client participating in the event.
    std::string party_id;
    //! The version of the VoIP specification this message adheres to.
    std::string version;

    friend void from_json(const nlohmann::json &obj, CallReject &content);
    friend void to_json(nlohmann::json &obj, const CallReject &content);
};

//! Content for the `m.call.negotiate` event. See MSC2746.
struct CallNegotiate
{
    //! The ID of the call this event relates to.
    std::string call_id;
    //! A unique identifier for the client participating in the event.
    std::string party_id;
    //! The time in milliseconds that the negotiation is valid for. Recommended 90000ms.
    uint32_t lifetime = 90000;
    //! The session description object
    RTCSessionDescriptionInit description;

    friend void from_json(const nlohmann::json &obj, CallNegotiate &content);
    friend void to_json(nlohmann::json &obj, const CallNegotiate &content);
};

} // namespace mtx::events::voip
}
}
