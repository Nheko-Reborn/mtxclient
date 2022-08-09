#pragma once

/// @file
/// @brief Space related events to make child and parent relations

#include <optional>

#if __has_include(<nlohmann/json_fwd.hpp>)
#include <nlohmann/json_fwd.hpp>
#else
#include <nlohmann/json.hpp>
#endif

namespace mtx {
namespace events {
namespace state {
//! Namespace for space related state events.
namespace space {
/// @brief Event to point at a parent space from a room or space
///
/// To avoid abuse where a room admin falsely claims that a room is part of a space that it
/// should not be, clients could ignore such m.space.parent events unless either (a) there
/// is a corresponding m.space.child event in the claimed parent, or (b) the sender of the
/// m.space.child event has a sufficient power-level to send such an m.space.child event in
/// the parent. (It is not necessarily required that that user currently be a member of the
/// parent room - only the m.room.power_levels event is inspected.)
struct Parent
{
    /// @brief Servers to join the parent space via.
    ///
    /// Needs to contain at least one server.
    std::optional<std::vector<std::string>> via;
    /// @brief Determines whether this is the main parent for the space.
    ///
    /// When a user joins a room with a canonical parent, clients may switch to view the room in
    /// the context of that space, peeking into it in order to find other rooms and group them
    /// together. In practice, well behaved rooms should only have one canonical parent, but
    /// given this is not enforced: if multiple are present the client should select the one
    /// with the lowest room ID, as determined via a lexicographic ordering of the Unicode
    /// code-points.
    bool canonical = false;

    friend void from_json(const nlohmann::json &obj, Parent &child);
    friend void to_json(nlohmann::json &obj, const Parent &child);
};

/// @brief Event to point at a child room or space from a parent space
///
/// The admins of a space can advertise rooms and subspaces for their space by setting m.space.child
/// state events. The state_key is the ID of a child room or space, and the content must contain a
/// via key which gives a list of candidate servers that can be used to join the room.
struct Child
{
    /// @brief Servers to join the child room/space via.
    ///
    /// Needs to contain at least one server.
    std::optional<std::vector<std::string>> via;
    /// @brief A string which is used to provide a default ordering of siblings in the room
    /// list.
    ///
    /// Rooms are sorted based on a lexicographic ordering of the Unicode codepoints of the
    /// characters in order values. Rooms with no order come last, in ascending numeric order of
    /// the origin_server_ts of their m.room.create events, or ascending lexicographic order of
    /// their room_ids in case of equal origin_server_ts. orders which are not strings, or do
    /// not consist solely of ascii characters in the range \x20 (space) to \x7E (~), or consist
    /// of more than 50 characters, are forbidden and the field should be ignored if received.
    std::optional<std::string> order;

    //! Optional (default false) flag to denote whether the child is “suggested” or of interest to
    //! members of the space. This is primarily intended as a rendering hint for clients to display
    //! the room differently, such as eagerly rendering them in the room list.
    bool suggested = false;

    friend void from_json(const nlohmann::json &obj, Child &child);
    friend void to_json(nlohmann::json &obj, const Child &child);
};
}
}
}
}
