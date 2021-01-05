#pragma once

#include <vector>
#include <string>

#if __has_include(<nlohmann/json_fwd.hpp>)
#include <nlohmann/json_fwd.hpp>
#else
#include <nlohmann/json.hpp>
#endif

namespace mtx {
namespace responses {

//! Whether or not the room will be visible by non members.
enum class Visibility
{
        //! A private visibility will hide the room from the published room list.
        Private,
        //! Indicates that the room will be shown in the published room list
        Public,
};

inline
std::string visibilityToString(Visibility visibility)
{
        if (visibility == Visibility::Private) {
                return "private";
        }

        return "public";
}

inline
Visibility stringToVisibility(const std::string &s) {
    if (s == "private") {
        return Visibility::Private;
    }
    return Visibility::Public;
}

//! Response from the `GET /_matrix/client/r0/directory/list/room/{roomId}`endpoint.
struct RoomVisibility {
    //! The visibility of the room in the directory. One of: ["private", "public"]
    Visibility visibility;
};

void
from_json(const nlohmann::json &obj, RoomVisibility &res);

struct PublicRoomsChunk
{
    //! Aliases of the room. May be empty.
    std::vector<std::string> aliases;
    //! The canonical alias of the room, if any.
    std::string canonical_alias = "";
    //! The name of the room, if any.
    std::string name;
    //! **Required.** The number of members joined to the room.
    int num_joined_members;
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
};

void
from_json(const nlohmann::json &obj, PublicRoomsChunk &res);

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
        int total_room_count_estimate;
};

void
from_json(const nlohmann::json &obj, PublicRooms &publicRooms);

} // namespace responses
} // namespace mtx