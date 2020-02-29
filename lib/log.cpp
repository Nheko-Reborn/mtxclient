#include <mtx/log.hpp>

#include <iostream>

namespace mtx::utils::log {
void
log_warning(const std::string_view &msg)
{
        std::cerr << "warning:" << msg << "\n";
}

void
log_error(const std::string_view &msg)
{
        std::cerr << "error:" << msg << "\n";
}
}
