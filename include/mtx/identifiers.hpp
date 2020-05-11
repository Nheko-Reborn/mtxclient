#pragma once

#if __has_include(<nlohmann/json_fwd.hpp>)
#include <nlohmann/json_fwd.hpp>
#else
#include <nlohmann/json.hpp>
#endif

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
        std::string localpart() const { return localpart_; }
        //! Returns the name of the originating homeserver.
        std::string hostname() const { return hostname_; }
        //! Returns the whole identifier (localpart + hostname).
        std::string to_string() const { return id_; }

protected:
        //! Local part of the identifier.
        std::string localpart_;
        //! The name of the originating homeserver.
        std::string hostname_;
        //! The whole identifier.
        std::string id_;
};

class Event : public ID
{
public:
        template<typename Identifier>
        friend Identifier parse(const std::string &id);

private:
        //! The `sigil` used to represent an Event.
        std::string sigil = "$";
};

class Room : public ID
{
public:
        template<typename Identifier>
        friend Identifier parse(const std::string &id);

private:
        std::string sigil = "!";
};

class User : public ID
{
public:
        template<typename Identifier>
        friend Identifier parse(const std::string &id);
        friend bool operator<(const User &a, const User &b) { return a.id_ < b.id_; }

private:
        std::string sigil = "@";
};

//! Parses the given string into a @p Identifier.
//! \param id String to parse.
//! \returns The parsed @p Identifier.
//! \throws std::invalid_argument in case of invalid input.
template<typename Identifier>
Identifier
parse(const std::string &id)
{
        Identifier identifier;

        if (id.empty()) {
                // FIXME: enable logging only in debug mode.
                /* std::cout << "mtx::identifiers - Empty matrix identifier was given" << std::endl;
                 */
                return identifier;
        }

        if (std::string(1, id.at(0)) != identifier.sigil)
                throw std::invalid_argument(
                  std::string(id + ": missing sigil " + identifier.sigil + "\n"));

        const auto parts = id.find_first_of(':');

        // Split into localpart and server.
        if (parts != std::string::npos) {
                identifier.localpart_ = id.substr(1, parts - 1);
                identifier.hostname_  = id.substr(parts + 1);
                identifier.id_        = id;
        } else {
                // V3 event ids don't use ':' at all, don't parse them the same way.
                identifier.localpart_ = id;
                identifier.hostname_  = id;
                identifier.id_        = id;
        }

        return identifier;
}

void
from_json(const nlohmann::json &obj, User &user);

void
to_json(nlohmann::json &obj, const User &user);

void
from_json(const nlohmann::json &obj, Room &room)

  ;

void
to_json(nlohmann::json &obj, const Room &room);

void
from_json(const nlohmann::json &obj, Event &event);

void
to_json(nlohmann::json &obj, const Event &event);

} // namespace identifiers
} // namespace mtx
