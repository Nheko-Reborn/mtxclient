#pragma once

#if __has_include(<nlohmann/json_fwd.hpp>)
#include <nlohmann/json_fwd.hpp>
#else
#include <nlohmann/json.hpp>
#endif

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
from_json(const nlohmann::json &obj, Encryption &encryption);

void
to_json(nlohmann::json &obj, const Encryption &encryption);

} // namespace state
} // namespace events
} // namespace mtx
