#pragma once

#include <nlohmann/json.hpp>

#include <string>
#include <vector>

#include "mtx/events/collections.hpp"

using json = nlohmann::json;

namespace mtx {
namespace responses {

struct EventId
{
        mtx::identifiers::Event event_id;
};

void
from_json(const nlohmann::json &obj, EventId &response);

struct GroupId
{
        std::string group_id;
};

void
from_json(const nlohmann::json &obj, GroupId &response);

struct FilterId
{
        std::string filter_id;
};

void
from_json(const nlohmann::json &obj, FilterId &response);

namespace utils {

using RoomAccountDataEvents = std::vector<mtx::events::collections::RoomAccountDataEvents>;
using TimelineEvents        = std::vector<mtx::events::collections::TimelineEvents>;
using StateEvents           = std::vector<mtx::events::collections::StateEvents>;
using StrippedEvents        = std::vector<mtx::events::collections::StrippedEvents>;

namespace states = mtx::events::state;
namespace msgs   = mtx::events::msg;

struct TimelineEventVisitor
{
        json operator()(const events::StateEvent<states::Aliases> &aliasEv) const
        {
                json j;
                mtx::events::to_json(j, aliasEv);
                return j;
        }
        json operator()(const events::StateEvent<states::Avatar> &avatarEv) const
        {
                json j;
                mtx::events::to_json(j, avatarEv);
                return j;
        };
        json operator()(const events::StateEvent<states::CanonicalAlias> &aliasEv) const
        {
                json j;
                mtx::events::to_json(j, aliasEv);
                return j;
        };
        json operator()(const events::StateEvent<states::Create> &createEv) const
        {
                json j;
                mtx::events::to_json(j, createEv);
                return j;
        };
        json operator()(const events::StateEvent<states::Encryption> &encEv) const
        {
                json j;
                mtx::events::to_json(j, encEv);
                return j;
        };
        json operator()(const events::StateEvent<states::GuestAccess> &guestEv) const
        {
                json j;
                mtx::events::to_json(j, guestEv);
                return j;
        };
        json operator()(const events::StateEvent<states::HistoryVisibility> &histEv) const
        {
                json j;
                mtx::events::to_json(j, histEv);
                return j;
        };
        json operator()(const events::StateEvent<states::JoinRules> &joinEv) const
        {
                json j;
                mtx::events::to_json(j, joinEv);
                return j;
        };
        json operator()(const events::StateEvent<states::Member> &membEv) const
        {
                json j;
                mtx::events::to_json(j, membEv);
                return j;
        };
        json operator()(const events::StateEvent<states::Name> &nameEv) const
        {
                json j;
                mtx::events::to_json(j, nameEv);
                return j;
        };
        json operator()(const events::StateEvent<states::PinnedEvents> &pinEv) const
        {
                json j;
                mtx::events::to_json(j, pinEv);
                return j;
        };
        json operator()(const events::StateEvent<states::PowerLevels> &powEv) const
        {
                json j;
                mtx::events::to_json(j, powEv);
                return j;
        };
        json operator()(const events::StateEvent<states::Tombstone> &tombEv) const
        {
                json j;
                mtx::events::to_json(j, tombEv);
                return j;
        };
        json operator()(const events::StateEvent<states::Topic> &topicEv) const
        {
                json j;
                mtx::events::to_json(j, topicEv);
                return j;
        };
        json operator()(const events::EncryptedEvent<msgs::Encrypted> &encEv) const
        {
                json j;
                mtx::events::to_json(j, encEv);
                return j;
        };
        json operator()(const events::RedactionEvent<msgs::Redaction> &redEv) const
        {
                json j;
                mtx::events::to_json(j, redEv);
                return j;
        };
        json operator()(const events::Sticker &stickEv) const
        {
                json j;
                mtx::events::to_json(j, stickEv);
                return j;
        };
        json operator()(const events::RoomEvent<msgs::Redacted> &redEv) const
        {
                json j;
                mtx::events::to_json(j, redEv);
                return j;
        };
        json operator()(const events::RoomEvent<msgs::Audio> &audioEv) const
        {
                json j;
                mtx::events::to_json(j, audioEv);
                return j;
        };
        json operator()(const events::RoomEvent<msgs::Emote> &emoteEv) const
        {
                json j;
                mtx::events::to_json(j, emoteEv);
                return j;
        };
        json operator()(const events::RoomEvent<msgs::File> &fileEv) const
        {
                json j;
                mtx::events::to_json(j, fileEv);
                return j;
        };
        json operator()(const events::RoomEvent<msgs::Image> &imageEv) const
        {
                json j;
                mtx::events::to_json(j, imageEv);
                return j;
        };
        // TODO: json operator()(const events::RoomEvent<msgs::Location> &locEv) const { json j;
        // mtx::events::to_json(j, locEv); return j;};
        json operator()(const events::RoomEvent<msgs::Notice> &noticeEv) const
        {
                json j;
                mtx::events::to_json(j, noticeEv);
                return j;
        };
        json operator()(const events::RoomEvent<msgs::Text> &textEv) const
        {
                json j;
                mtx::events::to_json(j, textEv);
                return j;
        };
        json operator()(const events::RoomEvent<msgs::Video> &videoEv) const
        {
                json j;
                mtx::events::to_json(j, videoEv);
                return j;
        };
};

void
log_error(json::exception &err, const json &event);

void
log_error(std::string err, const json &event);

void
parse_room_account_data_events(const json &events, RoomAccountDataEvents &container);

void
compose_timeline_events(json &events, const TimelineEvents &container);

void
parse_timeline_events(const json &events, TimelineEvents &container);

void
parse_state_events(const json &events, StateEvents &container);

void
parse_stripped_events(const json &events, StrippedEvents &container);
}
}
}
