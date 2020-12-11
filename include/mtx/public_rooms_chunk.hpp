#pragma once

#include <string>
#include <vector>

#if __has_include(<nlohmann/json_fwd.hpp>)
#include <nlohmann/json_fwd.hpp>
#else
#include <nlohmann/json.hpp>
#endif

namespace mtx {
namespace responses {
struct PublicRoomsChunk
{
    //! Aliases of the room. May be empty.
    std::vector<std::string> aliases;
    //! The canonical alias of the room, if any.
    std::string canonical_alias
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

void
to_json(nlohmann::json &obj, const PublicRoomsChunk &res);

} // namespace responses
} // namespace mtx