#pragma once

#include <string_view>

namespace mtx::utils::log {
void
log_warning(const std::string_view &msg);
void
log_error(const std::string_view &msg);
}
