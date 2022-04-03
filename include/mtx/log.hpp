#pragma once

#include <memory>
#include <spdlog/spdlog.h>

namespace mtx {
namespace utils {
namespace log {
std::shared_ptr<spdlog::logger>
log();
}
}
}
