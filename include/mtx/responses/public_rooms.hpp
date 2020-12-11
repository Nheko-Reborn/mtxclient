#pragma once

#include <vector>
#include <string>

#if __has_include(<nlohmann/json_fwd.hpp>)
#include <nlohmann/json_fwd.hpp>
#else
#include <nlohmann/json.hpp>
#endif

#include "mtx/public_rooms_chunk.hpp"

namespace mtx {
namespace responses {

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