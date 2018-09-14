#pragma once

#include <nlohmann/json.hpp>
#include <string>

using json = nlohmann::json;

namespace mtx {
namespace errors {

enum class ErrorCode
{
        M_UNRECOGNIZED,
        //! Forbidden access, e.g. joining a room without permission, failed login.
        M_FORBIDDEN,
        //! The access token specified was not recognised.
        M_UNKNOWN_TOKEN,
        //! Request contained valid JSON, but it was malformed in some way,
        //! e.g. missing required keys, invalid values for keys
        M_BAD_JSON,
        //! Request did not contain valid JSON.
        M_NOT_JSON,
        //! No resource was found for this request.
        M_NOT_FOUND,
        //! Too many requests have been sent in a short period of time.
        M_LIMIT_EXCEEDED,
        //! Encountered when trying to register a user ID which has been taken.
        M_USER_IN_USE,
        //! Encountered when trying to register a user ID which is not valid.
        M_INVALID_USERNAME,
        //! Sent when the room alias given to the createRoom API is already in use.
        M_ROOM_IN_USE,
        //! Sent when the intial state given to the createRoom API is invalid.
        M_INVALID_ROOM_STATE,
        //! Encountered when specifying bad pagination query parameters.
        M_BAD_PAGINATION,
        //! Sent when a threepid given to an API cannot be used because
        //! the same threepid is already in use.
        M_THREEPID_IN_USE,
        //! Sent when a threepid given to an API cannot be used
        //! because no record matching the threepid was found.
        M_THREEPID_NOT_FOUND,
        //! The client's request used a third party server,
        //! eg. ID server, that this server does not trust.
        M_SERVER_NOT_TRUSTED,
        //! The access token isn't present in the request.
        M_MISSING_TOKEN,
};

std::string
to_string(ErrorCode code);

ErrorCode
from_string(const std::string &code);

//! Represents a Matrix related error.
struct Error
{
        //! Error code.
        ErrorCode errcode;
        //! Human readable version of the error.
        std::string error;
};

void
from_json(const json &obj, Error &error);
}
}
