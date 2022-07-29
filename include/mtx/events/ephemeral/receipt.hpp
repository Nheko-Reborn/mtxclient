#pragma once

/// @file
/// @brief Read notifications

#include <map>
#include <string>

#if __has_include(<nlohmann/json_fwd.hpp>)
#include <nlohmann/json_fwd.hpp>
#else
#include <nlohmann/json.hpp>
#endif

namespace mtx {
namespace events {
namespace ephemeral {

//! An individual receipt, which specifies when the event was read by this user.
struct IndividualReceipt
{
    //! The timestamp the receipt was sent at.
    uint64_t ts = 0;
};

//! A list of receipts for a single event.
struct Receipts
{
    //! The mapping of user ID to receipt. The user ID is the entity who sent this receipt.
    std::map<std::string, IndividualReceipt> users;
};

/// @brief Read notifications / `m.receipt`
///
/// These receipts are a form of acknowledgement of an event. This module defines a single
/// acknowledgement: m.read which indicates that the user has read up to a given event.
struct Receipt
{
    //! The type of read receipt, currently public or private
    enum ReceiptType
    {
        //! A public read receipt (m.read)
        Read,
        //! A private read receipt (MSC2285)
        ReadPrivate
    };
    //! The mapping of event ID to a collection of receipts for this event ID. The event ID is
    //! the ID of the event being acknowledged and not an ID for the receipt itself.
    std::map<std::string, std::map<ReceiptType, Receipts>> receipts;

    //! Deserialization method needed by @p nlohmann::json.
    friend void from_json(const nlohmann::json &obj, Receipt &content);

    //! Serialization method needed by @p nlohmann::json.
    friend void to_json(nlohmann::json &obj, const Receipt &content);
};

} // namespace state
} // namespace events
} // namespace mtx
