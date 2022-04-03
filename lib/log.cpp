#include <mtx/log.hpp>

#include "spdlog/sinks/stdout_color_sinks.h"

namespace mtx::utils::log {
std::shared_ptr<spdlog::logger>
log()
{
    static auto mtx_logger = std::make_shared<spdlog::logger>(
      "mtx", std::make_shared<spdlog::sinks::stderr_color_sink_mt>());

    return mtx_logger;
}
}
