#include "mtx/errors.hpp"
#include "mtxclient/http/errors.hpp"

#include <coeurl/errors.hpp>

#include <nlohmann/json.hpp>

namespace mtx::errors {
std::string
to_string(ErrorCode code)
{
    switch (code) {
    case ErrorCode::M_UNRECOGNIZED:
        return "M_UNRECOGNIZED";
    case ErrorCode::M_FORBIDDEN:
        return "M_FORBIDDEN";
    case ErrorCode::M_UNKNOWN_TOKEN:
        return "M_UNKNOWN_TOKEN";
    case ErrorCode::M_BAD_JSON:
        return "M_BAD_JSON";
    case ErrorCode::M_NOT_JSON:
        return "M_NOT_JSON";
    case ErrorCode::M_NOT_FOUND:
        return "M_NOT_FOUND";
    case ErrorCode::M_LIMIT_EXCEEDED:
        return "M_LIMIT_EXCEEDED";
    case ErrorCode::M_USER_IN_USE:
        return "M_USER_IN_USE";
    case ErrorCode::M_INVALID_USERNAME:
        return "M_INVALID_USERNAME";
    case ErrorCode::M_ROOM_IN_USE:
        return "M_ROOM_IN_USE";
    case ErrorCode::M_INVALID_ROOM_STATE:
        return "M_INVALID_ROOM_STATE";
    case ErrorCode::M_BAD_PAGINATION:
        return "M_BAD_PAGINATION";
    case ErrorCode::M_THREEPID_IN_USE:
        return "M_THREEPID_IN_USE";
    case ErrorCode::M_THREEPID_NOT_FOUND:
        return "M_THREEPID_NOT_FOUND";
    case ErrorCode::M_SERVER_NOT_TRUSTED:
        return "M_SERVER_NOT_TRUSTED";
    case ErrorCode::M_MISSING_TOKEN:
        return "M_MISSING_TOKEN";
    case ErrorCode::M_INVALID_SIGNATURE:
        return "M_INVALID_SIGNATURE";
    case ErrorCode::M_EXCLUSIVE:
        return "M_EXCLUSIVE";
    case ErrorCode::M_UNKNOWN:
        return "M_UNKNOWN";
    }

    return "";
}

ErrorCode
from_string(const std::string &code)
{
    if (code == "M_FORBIDDEN")
        return ErrorCode::M_FORBIDDEN;
    else if (code == "M_UNKNOWN_TOKEN")
        return ErrorCode::M_UNKNOWN_TOKEN;
    else if (code == "M_BAD_JSON")
        return ErrorCode::M_BAD_JSON;
    else if (code == "M_NOT_JSON")
        return ErrorCode::M_NOT_JSON;
    else if (code == "M_NOT_FOUND")
        return ErrorCode::M_NOT_FOUND;
    else if (code == "M_LIMIT_EXCEEDED")
        return ErrorCode::M_LIMIT_EXCEEDED;
    else if (code == "M_USER_IN_USE")
        return ErrorCode::M_USER_IN_USE;
    else if (code == "M_INVALID_USERNAME")
        return ErrorCode::M_INVALID_USERNAME;
    else if (code == "M_ROOM_IN_USE")
        return ErrorCode::M_ROOM_IN_USE;
    else if (code == "M_INVALID_ROOM_STATE")
        return ErrorCode::M_INVALID_ROOM_STATE;
    else if (code == "M_BAD_PAGINATION")
        return ErrorCode::M_BAD_PAGINATION;
    else if (code == "M_THREEPID_IN_USE")
        return ErrorCode::M_THREEPID_IN_USE;
    else if (code == "M_THREEPID_NOT_FOUND")
        return ErrorCode::M_THREEPID_NOT_FOUND;
    else if (code == "M_SERVER_NOT_TRUSTED")
        return ErrorCode::M_SERVER_NOT_TRUSTED;
    else if (code == "M_MISSING_TOKEN")
        return ErrorCode::M_MISSING_TOKEN;
    else if (code == "M_INVALID_SIGNATURE")
        return ErrorCode::M_INVALID_SIGNATURE;
    else if (code == "M_EXCLUSIVE")
        return ErrorCode::M_EXCLUSIVE;
    else if (code == "M_UNKNOWN")
        return ErrorCode::M_UNKNOWN;
    else // if (code == "M_UNRECOGNIZED")
        return ErrorCode::M_UNRECOGNIZED;
}

void
from_json(const nlohmann::json &obj, LightweightError &error)
{
    error.errcode = from_string(obj.value("errcode", ""));
    error.error   = obj.value("error", "");
}

void
from_json(const nlohmann::json &obj, Error &error)
{
    error.errcode = from_string(obj.value("errcode", ""));
    error.error   = obj.value("error", "");

    if (obj.contains("flows"))
        error.unauthorized = obj.get<user_interactive::Unauthorized>();

    if (obj.contains("retry_after_ms"))
        error.retry_after =
          std::chrono::milliseconds(obj.value("retry_after_ms", std::uint64_t{0}));
}
}

const char *
mtx::http::ClientError::error_code_string() const
{
    return coeurl::to_string(static_cast<CURLcode>(error_code));
}
