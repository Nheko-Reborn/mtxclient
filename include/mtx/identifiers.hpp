#pragma once

/// @file
/// @brief Identifiers used in the Matrix API.

#if __has_include(<nlohmann/json_fwd.hpp>)
#include <nlohmann/json_fwd.hpp>
#else
#include <nlohmann/json.hpp>
#endif

#include <compare>
#include <stdexcept>

namespace mtx {
namespace identifiers {

//! Base class for all the identifiers.
//
//! Each identifier has the following format `(sigil)``(localpart)`:`(hostname)`.
class ID
{
public:
    //! Returns the unique local part of the identifier.
    [[nodiscard]] std::string localpart() const { return localpart_; }
    //! Returns the name of the originating homeserver.
    [[nodiscard]] std::string hostname() const { return hostname_; }
    //! Returns the whole identifier (localpart + hostname).
    [[nodiscard]] std::string to_string() const { return id_; }

protected:
    //! Local part of the identifier.
    std::string localpart_;
    //! The name of the originating homeserver.
    std::string hostname_;
    //! The whole identifier.
    std::string id_;
};

//! An event id.
class Event : public ID
{
public:
    template<typename Identifier>
    friend Identifier parse(const std::string &id);

private:
    //! The `sigil` used to represent an Event.
    static constexpr std::string_view sigil = "$";

    friend void from_json(const nlohmann::json &obj, Event &event);
    friend void to_json(nlohmann::json &obj, const Event &event);
};

//! A room id.
class Room : public ID
{
public:
    template<typename Identifier>
    friend Identifier parse(const std::string &id);

private:
    static constexpr std::string_view sigil = "!";

    friend void from_json(const nlohmann::json &obj, Room &room);
    friend void to_json(nlohmann::json &obj, const Room &room);
};

//! A user id.
class User : public ID
{
public:
    template<typename Identifier>
    friend Identifier parse(const std::string &id);
    auto operator<=>(User const &other) const noexcept { return id_.compare(other.id_) <=> 0; };
    bool operator==(User const &other) const noexcept { return id_ == other.id_; };

private:
    static constexpr std::string_view sigil = "@";

    friend void from_json(const nlohmann::json &obj, User &user);
    friend void to_json(nlohmann::json &obj, const User &user);
};

//! Parses the given string into a @p Identifier.
//! \param id String to parse.
//! \returns The parsed @p Identifier.
//! \throws std::invalid_argument in case of invalid input.
template<typename Identifier>
Identifier
parse(const std::string &id)
{
    if (id.empty()) {
        // FIXME: enable logging only in debug mode.
        /* std::cout << "mtx::identifiers - Empty matrix identifier was given" << std::endl;
         */
        return {};
    }

    if (std::string(1, id.at(0)) != Identifier::sigil)
        throw std::invalid_argument(id + ": missing sigil " + std::string(Identifier::sigil));

    const auto parts = id.find_first_of(':');

    // Split into localpart and server.
    if (parts != std::string::npos) {
        Identifier identifier{};
        identifier.localpart_ = id.substr(1, parts - 1);
        identifier.hostname_  = id.substr(parts + 1);
        identifier.id_        = id;
        return identifier;
    } else if (Identifier::sigil == "$") {
        // V3 event ids don't use ':' at all, don't parse them the same way.
        Identifier identifier{};
        identifier.localpart_ = id;
        identifier.hostname_  = id;
        identifier.id_        = id;
        return identifier;
    } else {
        throw std::invalid_argument(id + ": invalid id");
    }
}

} // namespace identifiers
} // namespace mtx
