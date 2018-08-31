#pragma once

#include <json.hpp>
#include <string>

namespace mtx {
namespace responses {

//! Represents the response of `POST /_matrix/media/r0/upload`
struct ContentURI
{
        //! The MXC URI for the uploaded content.
        std::string content_uri;
};

void
from_json(const nlohmann::json &obj, ContentURI &response);
}
}
