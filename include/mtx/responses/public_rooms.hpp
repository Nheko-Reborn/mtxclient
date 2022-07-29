#pragma once

#include <optional>
#include <string>
#include <vector>

#include "mtx/common.hpp"
#include "mtx/events/collections.hpp"

#if __has_include(<nlohmann/json_fwd.hpp>)
#include <nlohmann/json_fwd.hpp>
#else
#include <nlohmann/json.hpp>
#endif

namespace common = mtx::common;

namespace mtx {
namespace responses {
//! Response from the `GET /_matrix/client/r0/directory/list/room/{roomId}`endpoint.
struct PublicRoomVisibility
{
    //! The visibility of the room in the directory. One of: ["private", "public"]
    common::RoomVisibility visibility;

    friend void from_json(const nlohmann::json &obj, PublicRoomVisibility &res);
};

struct PublicRoomsChunk
{
    //! Aliases of the room. May be empty.
    std::vector<std::string> aliases;
    //! The canonical alias of the room, if any.
    std::string canonical_alias = "";
    //! The name of the room, if any.
    std::string name;
    //! **Required.** The number of members joined to the room.
    size_t num_joined_members;
    //! **Required.** The ID of the room.
    std::string room_id;
    //! The topic of the room, if any.
    std::string topic;
    //! **Required.** Whether the room may be viewed by guest users without joining.
    bool world_readable;
    //! **Required.** Whether guest users may join the room
    //! and participate in it. If they can, they will be subject
    //! to ordinary power level rules like any other user.
    bool guest_can_join;
    //! The URL for the room's avatar, if one is set.
    std::string avatar_url;

    //! The room’s join rule. When not present, the room is assumed to be public.
    mtx::events::state::JoinRule join_rule = mtx::events::state::JoinRule::Public;

    //! Required: The type of room (from m.room.create), if any.
    std::string room_type;

    //! Optional (not all APIs return this): The version of this room.
    std::string room_version;

    //! Optional: If you are a member of this room. Can usually be assumed to be "leave".
    events::state::Membership membership = events::state::Membership::Leave;

    //! Optional: If the room is encrypted, the encryption algorithm.
    std::string encryption;

    //! The m.space.child events of the space-room, represented as Stripped State Events with an
    //! added origin_server_ts key.
    //!
    //! If the room is not a space-room, this should be empty.
    std::vector<mtx::events::collections::StrippedEvents> children_state;

    friend void from_json(const nlohmann::json &obj, PublicRoomsChunk &res);
};
using PublicRoom = PublicRoomsChunk;

//! Response from the `GET /_matrix/client/r0/publicRooms` &
//! `POST /_matrix/client/r0/publicRooms` endpoints.
struct PublicRooms
{
    //! **Required**. A paginated chunk of public rooms.
    std::vector<PublicRoomsChunk> chunk;
    //! A pagination token for the response. The absence
    //! of this token means there are no more results to
    //! fetch and the client should stop paginating.
    std::string next_batch;
    //! A pagination token that allows fetching previous results.
    //! The absence of this token means there are no results
    //! before this batch, i.e. this is the first batch.
    std::string prev_batch;
    //! An estimate on the total number of public rooms,
    //! if the server has an estimate.
    std::optional<size_t> total_room_count_estimate;

    friend void from_json(const nlohmann::json &obj, PublicRooms &publicRooms);
};

//! Response from the `GET /_matrix/client/v1/rooms/{roomId}/hierarchy`
struct HierarchyRooms
{
    //! Required: The rooms for the current page, with the current filters.
    std::vector<PublicRoomsChunk> rooms;
    //! A token to supply to from to keep paginating the responses. Not present when there are no
    //! further results.
    std::string next_batch;

    friend void from_json(const nlohmann::json &obj, HierarchyRooms &publicRooms);
};

} // namespace responses
} // namespace mtx
