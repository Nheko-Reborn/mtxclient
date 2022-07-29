#pragma once

/// @file
/// @brief device related endpoints.

#if __has_include(<nlohmann/json_fwd.hpp>)
#include <nlohmann/json_fwd.hpp>
#else
#include <nlohmann/json.hpp>
#endif

#include "mtx/common.hpp"
#include "mtx/lightweight_error.hpp"

#include <string>
#include <vector>

namespace mtx {
namespace responses {

struct Device
{
    //! **Required.** Identifier of this device.
    std::string device_id;

    //! Display name set by the user for this device. Absent if no name has been set.
    std::string display_name;

    //! The IP address where this device was last seen. (May be a few minutes out of date, for
    //! efficiency reasons).
    std::string last_seen_ip;

    //! The timestamp (in milliseconds since the unix epoch) when this devices was last seen. (May
    //! be a few minutes out of date, for efficiency reasons).
    size_t last_seen_ts;

    friend void from_json(const nlohmann::json &obj, Device &res);
};

//! Response from the `GET /_matrix/client/r0/devices` endpoint.
struct QueryDevices
{
    //! Gets information about all devices for the current user.
    //! A list of all registered devices for this user.
    //
    std::vector<Device> devices;

    friend void from_json(const nlohmann::json &obj, QueryDevices &response);
};

}
}
