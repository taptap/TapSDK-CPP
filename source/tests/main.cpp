#include <iostream>

#include "base/logging.h"

int main() {
    std::cout << "Hello, World!" << std::endl;

    LOG_ERROR("Test: {}", "");
    return 0;
}
