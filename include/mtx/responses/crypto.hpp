#pragma once

#include <nlohmann/json.hpp>

#include "mtx/common.hpp"

#include <map>
#include <string>

namespace mtx {
namespace responses {
//! Response from the `POST /_matrix/client/r0/keys/upload` endpoint.
struct UploadKeys
{
        //! For each key algorithm, the number of unclaimed one-time keys
        //! of that type currently held on the server for this device.
        std::map<std::string, uint32_t> one_time_key_counts;
};

void
from_json(const nlohmann::json &obj, UploadKeys &response);

using DeviceToKeysMap = std::map<std::string, mtx::crypto::DeviceKeys>;

//! Response from the `POST /_matrix/client/r0/keys/query` endpoint.
struct QueryKeys
{
        //! If any remote homeservers could not be reached, they are
        //! recorded here. The names of the properties are the names
        //! of the unreachable servers.
        std::map<std::string, nlohmann::json> failures;
        //! Information on the queried devices.
        //! A map from user ID, to a map from device ID to device information.
        //! For each device, the information returned will be the same
        //! as uploaded via /keys/upload, with the addition of an unsigned property
        std::map<std::string, DeviceToKeysMap> device_keys;
};

void
from_json(const nlohmann::json &obj, QueryKeys &response);

//! Response from the `POST /_matrix/client/r0/keys/claim` endpoint.
struct ClaimKeys
{
        //! If any remote homeservers could not be reached, they are
        //! recorded here. The names of the properties are the names
        //! of the unreachable servers.
        std::map<std::string, nlohmann::json> failures;
        //! One-time keys for the queried devices. A map from user ID,
        //! to a map from <algorithm>:<key_id> to the key object.
        std::map<std::string, std::map<std::string, nlohmann::json>> one_time_keys;
};

inline void
from_json(const nlohmann::json &obj, ClaimKeys &response)
{
        response.failures = obj.at("failures").get<std::map<std::string, nlohmann::json>>();
        response.one_time_keys =
          obj.at("one_time_keys")
            .get<std::map<std::string, std::map<std::string, nlohmann::json>>>();
}

//! Response from the `GET /_matrix/client/r0/keys/changes` endpoint.
struct KeyChanges
{
        //! The Matrix User IDs of all users who updated their device identity keys.
        std::vector<std::string> changed;
        //! The Matrix User IDs of all users who may have left all the end-to-end
        //! encrypted rooms they previously shared with the user.
        std::vector<std::string> left;
};

inline void
from_json(const nlohmann::json &obj, KeyChanges &response)
{
        response.changed = obj.at("changed").get<std::vector<std::string>>();
        response.left    = obj.at("left").get<std::vector<std::string>>();
}

} // namespace responses
} // namespace mtx
