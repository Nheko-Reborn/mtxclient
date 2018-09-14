#pragma once

#include <nlohmann/json.hpp>
#include <string>

#include "mtx/events/common.hpp"

using json = nlohmann::json;

namespace common = mtx::common;

namespace mtx {
namespace events {
namespace msg {

struct File
{
        // A human-readable description of the file. This is recommended to
        // be the filename of the original upload.
        std::string body;
        // The original filename of the uploaded file.
        // SPEC_BUG: The filename is not really required.
        std::string filename;
        // Must be 'm.file'.
        std::string msgtype;
        // The matrix URL of the file.
        std::string url;
        // Information about the file referred to in the url.
        common::FileInfo info;
};

void
from_json(const json &obj, File &content);

void
to_json(json &obj, const File &content);

} // namespace msg
} // namespace events
} // namespace mtx
