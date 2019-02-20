#pragma once

#include <nlohmann/json.hpp>

using json = nlohmann::json;

namespace mtx {
namespace events {
namespace state {

//! Content of the `m.room.encryption` event.
//
//! A client can enable encryption to a room by sending this event.
struct Encryption
{
        //! Defines which encryption algorithm should be used for encryption.
        //! Currently only m.megolm.v1-aes-sha2 is permitted.
        std::string algorithm = "m.megolm.v1.aes-sha2";
};

void
from_json(const json &obj, Encryption &encryption);

void
to_json(json &obj, const Encryption &encryption);

} // namespace state
} // namespace events
} // namespace mtx
