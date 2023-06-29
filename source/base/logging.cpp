#include <iostream>
#include "logging.h"

namespace base::log {

static Level log_level = Level::Info;

void SetLogLevel(Level level) { log_level = level; }

void LogMessage(base::log::Level level, const std::string& message) {
    if (log_level >= level) {
        return;
    }
    std::cout << message << std::endl;
}

void AssertFailed(const std::string& message) {
    std::cerr << message << std::endl;
    abort();
}

}  // namespace base::log