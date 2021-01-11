#include <nlohmann/json.hpp>

#include "mtx/identifiers.hpp"
#include "mtx/responses/public_rooms.hpp"
#include "mtx/common.hpp"

namespace mtx {
namespace responses {

void
from_json(const nlohmann::json &obj, PublicRoomVisibility &res)
{
        res.visibility = mtx::common::stringToVisibility(obj.at("visibility").get<std::string>());
}

void
from_json(const nlohmann::json &obj, PublicRoomsChunk &res)
{
        res.aliases = obj.value("aliases", std::vector<std::string>{});

        res.canonical_alias = obj.value("canonical_alias", std::string{});

        res.name = obj.value("name", std::string{});

        res.num_joined_members = obj.at("num_joined_members").get<size_t>();

        res.room_id = obj.at("room_id").get<std::string>();

        res.topic = obj.value("topic", std::string{});

        res.world_readable = obj.at("world_readable").get<bool>();

        res.guest_can_join = obj.at("guest_can_join").get<bool>();

        res.avatar_url = obj.value("avatar_url", std::string{});
}

void
from_json(const nlohmann::json &obj, PublicRooms &publicRooms)
{
        // PublicRoomsChunk is CopyConstructible & DefaultConstructible
        publicRooms.chunk = obj.at("chunk").get<std::vector<PublicRoomsChunk>>();

        if (obj.count("next_batch")) {
                publicRooms.next_batch = obj.at("next_batch").get<std::string>();
        }

        if (obj.count("prev_batch")) {
                publicRooms.prev_batch = obj.at("prev_batch").get<std::string>();
        }

        publicRooms.total_room_count_estimate =
          obj.count("total_room_count_estimate")
            ? std::optional<size_t>{obj.at("total_room_count_estimate").get<size_t>()}
            : std::nullopt;
}

} // namespace responses
} // namespace mtx