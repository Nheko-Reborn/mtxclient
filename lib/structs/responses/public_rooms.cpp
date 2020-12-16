#include <nlohmann/json.hpp>

#include "mtx/identifiers.hpp"
#include "mtx/responses/public_rooms.hpp"

namespace mtx {
namespace responses {

void
from_json(const nlohmann::json &obj, PublicRoomsChunk &res) 
{
    if (obj.count("aliases") != 0 && !obj.at("aliases").is_null()) {
        res.aliases = obj.at("aliases").get<std::vector<std::string>>();
    }

    if (obj.count("canonical_alias") != 0 && !obj.at("canonical_alias").is_null()) {
        res.canonical_alias = obj.at("canonical_alias").get<std::string>();
    }

    if (obj.count("name") != 0 && !obj.at("name").is_null()) {
        res.name = obj.at("name").get<std::string>();
    }

    if (obj.count("num_joined_members") != 0 && !obj.at("num_joined_members").is_null()) {
        res.num_joined_members = obj.at("num_joined_members").get<int>();
    }

    if (obj.count("room_id") != 0 && !obj.at("room_id").is_null()) {
        res.room_id = obj.at("room_id").get<std::string>();
    }

    if (obj.count("topic") != 0 && !obj.at("topic").is_null()) {
        res.topic = obj.at("topic").get<std::string>();
    }

    if (obj.count("world_readable") != 0 && !obj.at("world_readable").is_null()) {
        res.world_readable = obj.at("world_readable").get<bool>();
    }

    if (obj.count("guest_can_join") != 0 && !obj.at("guest_can_join").is_null()) {
        res.guest_can_join = obj.at("guest_can_join").get<bool>();
    }

    if (obj.count("avatar_url") != 0 && !obj.at("avatar_url").is_null()) {
        res.avatar_url = obj.at("avatar_url").get<std::string>();
    }
}

void
from_json(const nlohmann::json &obj, PublicRooms &publicRooms)
{
    // PublicRoomsChunk is CopyConstructible & DefaultConstructible
    if (obj.count("chunk") != 0 && !obj.at("chunk").is_null()) {
        publicRooms.chunk = obj.at("chunk").get<std::vector<PublicRoomsChunk>>();
    }

    if (obj.count("next_batch") != 0 && !obj.at("next_batch").is_null()) {
        publicRooms.next_batch = obj.at("next_batch").get<std::string>();
    }

    if (obj.count("prev_batch") != 0 && !obj.at("prev_batch").is_null()) {
        publicRooms.prev_batch = obj.at("prev_batch").get<std::string>();
    }

    if (obj.count("total_room_count_estimate") != 0 && !obj.at("total_room_count_estimate").is_null()) {
        publicRooms.total_room_count_estimate = obj.at("total_room_count_estimate").get<int>();
    }
}

} // namespace responses
} // namespace mtx