#include <string>

#include "mtx/events/voip.hpp"

#include <nlohmann/json.hpp>

using json = nlohmann::json;

namespace {
std::string
version(const json &obj)
{
    auto v = obj.at("version");
    return v.is_number() ? "0" : v.get<std::string>();
}

void
add_version(json &obj, std::string_view version)
{
    if (version == "0")
        obj["version"] = 0;
    else
        obj["version"] = version;
}
}

namespace mtx::events::voip {

// RTC Session Description
void
from_json(const json &obj, RTCSessionDescriptionInit &content)
{
    content.sdp = obj.at("sdp").get<std::string>();
    if (obj.at("type").get<std::string>() == "answer")
        content.type = RTCSessionDescriptionInit::Type::Answer;
    else if (obj.at("type").get<std::string>() == "offer")
        content.type = RTCSessionDescriptionInit::Type::Offer;
}

void
to_json(json &obj, const RTCSessionDescriptionInit &content)
{
    obj["sdp"] = content.sdp;
    if (content.type == RTCSessionDescriptionInit::Type::Answer)
        obj["type"] = "answer";
    else if (content.type == RTCSessionDescriptionInit::Type::Offer)
        obj["type"] = "offer";
}

// m.call.invite
void
from_json(const json &obj, CallInvite &content)
{
    content.call_id  = obj.at("call_id").get<std::string>();
    content.offer    = obj.at("offer").get<RTCSessionDescriptionInit>();
    content.version  = version(obj);
    content.lifetime = obj.at("lifetime").get<uint32_t>();
    if (content.version != "0") {
        content.party_id = obj.at("party_id").get<std::string>();
        if (obj.contains("invitee"))
            content.invitee = obj.at("invitee").get<std::string>();
    }
}

void
to_json(json &obj, const CallInvite &content)
{
    obj["call_id"] = content.call_id;
    obj["offer"]   = content.offer;
    add_version(obj, content.version);
    obj["lifetime"] = content.lifetime;
    if (content.version != "0") {
        obj["party_id"] = content.party_id;
        obj["invitee"]  = content.invitee;
    }
}

// m.call.candidates
void
from_json(const json &obj, CallCandidates::Candidate &content)
{
    if (obj.contains("sdpMid"))
        content.sdpMid = obj.at("sdpMid").get<std::string>();
    if (obj.contains("sdpMLineIndex"))
        content.sdpMLineIndex = obj.at("sdpMLineIndex").get<uint16_t>();
    if (obj.contains("candidate"))
        content.candidate = obj.at("candidate").get<std::string>();
}

void
to_json(json &obj, const CallCandidates::Candidate &content)
{
    obj["sdpMid"]        = content.sdpMid;
    obj["sdpMLineIndex"] = content.sdpMLineIndex;
    obj["candidate"]     = content.candidate;
}

void
from_json(const json &obj, CallCandidates &content)
{
    content.call_id    = obj.at("call_id").get<std::string>();
    content.candidates = obj.at("candidates").get<std::vector<CallCandidates::Candidate>>();
    content.version    = version(obj);
    if (content.version != "0") {
        content.party_id = obj.at("party_id").get<std::string>();
    }
}

void
to_json(json &obj, const CallCandidates &content)
{
    obj["call_id"]    = content.call_id;
    obj["candidates"] = content.candidates;
    add_version(obj, content.version);
    if (content.version != "0") {
        obj["party_id"] = content.party_id;
    }
}

// m.call.answer
void
from_json(const json &obj, CallAnswer &content)
{
    content.call_id = obj.at("call_id").get<std::string>();
    content.answer  = obj.at("answer").get<RTCSessionDescriptionInit>();
    content.version = version(obj);
    if (content.version != "0") {
        content.party_id = obj.at("party_id").get<std::string>();
    }
}

void
to_json(json &obj, const CallAnswer &content)
{
    obj["call_id"] = content.call_id;
    obj["answer"]  = content.answer;
    add_version(obj, content.version);
    if (content.version != "0") {
        obj["party_id"] = content.party_id;
    }
}

// m.call.hangup
void
from_json(const json &obj, CallHangUp &content)
{
    content.call_id = obj.at("call_id").get<std::string>();
    content.version = version(obj);
    if (content.version != "0") {
        content.party_id = obj.at("party_id").get<std::string>();
    }
    if (obj.count("reason") == 0) {
        content.reason = CallHangUp::Reason::User;
    } else {
        if (obj.at("reason").get<std::string>() == "ice_failed")
            content.reason = CallHangUp::Reason::ICEFailed;
        else if (obj.at("reason").get<std::string>() == "invite_timeout")
            content.reason = CallHangUp::Reason::InviteTimeOut;
        else if (obj.at("reason").get<std::string>() == "ice_timeout")
            content.reason = CallHangUp::Reason::ICETimeOut;
        else if (obj.at("reason").get<std::string>() == "user_hangup")
            content.reason = CallHangUp::Reason::UserHangUp;
        else if (obj.at("reason").get<std::string>() == "user_media_failed")
            content.reason = CallHangUp::Reason::UserMediaFailed;
        else if (obj.at("reason").get<std::string>() == "user_busy")
            content.reason = CallHangUp::Reason::UserBusy;
        else if (obj.at("reason").get<std::string>() == "unknown_error")
            content.reason = CallHangUp::Reason::UnknownError;
    }
}

void
to_json(json &obj, const CallHangUp &content)
{
    obj["call_id"] = content.call_id;
    add_version(obj, content.version);
    if (content.version != "0") {
        obj["party_id"] = content.party_id;
    }
    if (content.reason == CallHangUp::Reason::ICEFailed)
        obj["reason"] = "ice_failed";
    else if (content.reason == CallHangUp::Reason::InviteTimeOut)
        obj["reason"] = "invite_timeout";
    else if (content.reason == CallHangUp::Reason::ICETimeOut)
        obj["reason"] = "ice_timeout";
    else if (content.reason == CallHangUp::Reason::UserHangUp)
        obj["reason"] = "user_hangup";
    else if (content.reason == CallHangUp::Reason::UserMediaFailed)
        obj["reason"] = "user_media_failed";
    else if (content.reason == CallHangUp::Reason::UserBusy)
        obj["reason"] = "user_busy";
    else if (content.reason == CallHangUp::Reason::UnknownError)
        obj["reason"] = "unknown_error";
}

// m.call.select_answer
void
from_json(const json &obj, CallSelectAnswer &content)
{
    content.call_id           = obj.at("call_id").get<std::string>();
    content.version           = version(obj);
    content.party_id          = obj.at("party_id").get<std::string>();
    content.selected_party_id = obj.at("selected_party_id").get<std::string>();
}

void
to_json(json &obj, const CallSelectAnswer &content)
{
    obj["call_id"] = content.call_id;
    add_version(obj, content.version);
    obj["party_id"]          = content.party_id;
    obj["selected_party_id"] = content.selected_party_id;
}

// m.call.reject
void
from_json(const json &obj, CallReject &content)
{
    content.call_id  = obj.at("call_id").get<std::string>();
    content.version  = version(obj);
    content.party_id = obj.at("party_id").get<std::string>();
}

void
to_json(json &obj, const CallReject &content)
{
    obj["call_id"] = content.call_id;
    add_version(obj, content.version);
    obj["party_id"] = content.party_id;
}

// m.call.negotiate
void
from_json(const json &obj, CallNegotiate &content)
{
    content.call_id     = obj.at("call_id").get<std::string>();
    content.party_id    = obj.at("party_id").get<std::string>();
    content.lifetime    = obj.at("lifetime").get<uint32_t>();
    content.description = obj.at("description");
}

void
to_json(json &obj, const CallNegotiate &content)
{
    obj["call_id"]     = content.call_id;
    obj["party_id"]    = content.party_id;
    obj["lifetime"]    = content.lifetime;
    obj["description"] = content.description;
}

} // namespace mtx::events::voip
