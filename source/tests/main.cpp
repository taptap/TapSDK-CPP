#include <iostream>

#include "base/logging.h"
#include "core/core.h"

int main() {
    std::cout << "Hello, World!" << std::endl;

    LOG_ERROR("Test: {}", "");
    return 0;
}
