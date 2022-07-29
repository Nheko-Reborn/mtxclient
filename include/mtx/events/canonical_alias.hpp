#pragma once

/// @file
/// @brief Change the canonical or listed avatars of a room.

#include <string>

#if __has_include(<nlohmann/json_fwd.hpp>)
#include <nlohmann/json_fwd.hpp>
#else
#include <nlohmann/json.hpp>
#endif

namespace mtx {
namespace events {
namespace state {

//! Content for the `m.room.canonical_alias` event.
//
//! This event is used to inform the room about which alias
//! should be considered the canonical one. This could be for
//! display purposes or as suggestion to users which alias to
//! use to advertise the room.
struct CanonicalAlias
{
    //! The canonical alias. Could be *null*.
    std::string alias;
    //! Alternative aliases the room advertises. This list can have aliases despite the alias
    //! field being null, empty, or otherwise not present.
    std::vector<std::string> alt_aliases;

    //! Deserialization method needed by @p nlohmann::json.
    friend void from_json(const nlohmann::json &obj, CanonicalAlias &canonical_alias);

    //! Serialization method needed by @p nlohmann::json.
    friend void to_json(nlohmann::json &obj, const CanonicalAlias &canonical_alias);
};

} // namespace state
} // namespace events
} // namespace mtx
