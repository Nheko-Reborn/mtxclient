#pragma once

/// @file
/// @brief Header for logging related functions

#include <memory>
#include <spdlog/spdlog.h>

namespace mtx {
namespace utils {
//! Logger utilities
namespace log {
/// @brief Access the logger and modify it.
///
/// For example you can set the sinks using `mtx::utils::log::log()->sinks() = sinks` or modify the
/// loglevel using `mtx::utils::log::log()->set_level(spdlog::level::trace)`.
std::shared_ptr<spdlog::logger>
log();
}
}
}
