#include <mtx/log.hpp>

#include "spdlog/sinks/stdout_color_sinks.h"

namespace mtx::utils::log {
std::shared_ptr<spdlog::logger>
log()
{
    static auto mtx_logger = spdlog::stderr_color_mt("mtx");

    return mtx_logger;
}
}
