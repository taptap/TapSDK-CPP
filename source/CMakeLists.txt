cmake_minimum_required(VERSION 3.21)
enable_language(ASM)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_EXTENSIONS ON)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Enable modules to include each other's files
include_directories(.)

add_subdirectory(base)
add_subdirectory(externals)
add_subdirectory(net)
add_subdirectory(sdk)
add_subdirectory(tests)
add_subdirectory(bindings)