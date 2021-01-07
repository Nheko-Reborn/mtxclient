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

namespace mtx::events::msg {

//! Content for the `m.call.invite` event.
struct CallInvite
{
        //! A unique identifier for the call.
        std::string call_id;
        //! The SDP text of the session description.
        std::string sdp;
        //! The version of the VoIP specification this message adheres to.
        uint16_t version;
        //! The time in milliseconds that the invite is valid for.
        uint32_t lifetime;
};

void
from_json(const nlohmann::json &obj, CallInvite &content);

void
to_json(nlohmann::json &obj, const CallInvite &content);

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
        //! Array of objects describing the candidates.
        std::vector<Candidate> candidates;
        //! The version of the VoIP specification this message adheres to.
        uint16_t version;
};

void
from_json(const nlohmann::json &obj, CallCandidates &content);

void
to_json(nlohmann::json &obj, const CallCandidates &content);

//! Content for the `m.call.answer` event
struct CallAnswer
{
        //! The ID of the call this event relates to.
        std::string call_id;
        //! The SDP text of the session description.
        std::string sdp;
        //! The version of the VoIP specification this message adheres to.
        uint16_t version;
};

void
from_json(const nlohmann::json &obj, CallAnswer &content);

void
to_json(nlohmann::json &obj, const CallAnswer &content);

//! Content for the `m.call.hangup` event
struct CallHangUp
{
        enum class Reason
        {
                ICEFailed,
                InviteTimeOut,
                User
        };

        //! The ID of the call this event relates to.
        std::string call_id;
        //! The version of the VoIP specification this message adheres to.
        uint16_t version;
        //! The reason for the call hang up.
        Reason reason = Reason::User;
};

void
from_json(const nlohmann::json &obj, CallHangUp &content);

void
to_json(nlohmann::json &obj, const CallHangUp &content);

} // namespace mtx::events::msg
