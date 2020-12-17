#include <nlohmann/json.hpp>

#include "mtx/identifiers.hpp"
#include "mtx/responses/public_rooms.hpp"

namespace mtx {
namespace responses {

void
from_json(const nlohmann::json &obj, PublicRoomsChunk &res) 
{
    res.aliases = obj.at("aliases").get<std::vector<std::string>>();

    res.canonical_alias = obj.at("canonical_alias").get<std::string>();

    res.name = obj.at("name").get<std::string>();

    res.num_joined_members = obj.at("num_joined_members").get<int>();

    res.room_id = obj.at("room_id").get<std::string>();

    res.topic = obj.at("topic").get<std::string>();

    res.world_readable = obj.at("world_readable").get<bool>();

    res.guest_can_join = obj.at("guest_can_join").get<bool>();

    res.avatar_url = obj.at("avatar_url").get<std::string>();
}

void
from_json(const nlohmann::json &obj, PublicRooms &publicRooms)
{
    // PublicRoomsChunk is CopyConstructible & DefaultConstructible
    publicRooms.chunk = obj.at("chunk").get<std::vector<PublicRoomsChunk>>();
    
    publicRooms.next_batch = obj.at("next_batch").get<std::string>();

    publicRooms.prev_batch = obj.at("prev_batch").get<std::string>();

    publicRooms.total_room_count_estimate = obj.at("total_room_count_estimate").get<int>();
}

} // namespace responses
} // namespace mtx