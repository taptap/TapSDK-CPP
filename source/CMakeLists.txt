cmake_minimum_required(VERSION 3.21)
enable_language(ASM)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_EXTENSIONS ON)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

message("CMAKE_BUILD_TYPE ${CMAKE_BUILD_TYPE}")
if (CMAKE_BUILD_TYPE STREQUAL "Release")
    set(CMAKE_VISIBILITY_INLINES_HIDDEN 1)
    add_definitions(-DOPENSSL_NO_ERR=1)
    add_definitions(-DOPENSSL_NO_FILENAMES=1)
    add_definitions(-DNDEBUG=1)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O3 -fdiscard-value-names -fno-show-source-location -fno-caret-diagnostics")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -O3 -fvisibility=hidden -fdiscard-value-names -fno-show-source-location -fno-caret-diagnostics")
    set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -s")
endif ()

# Enable modules to include each other's files
include_directories(.)

add_subdirectory(base)
add_subdirectory(externals)
add_subdirectory(core)
add_subdirectory(net)
add_subdirectory(sdk)
add_subdirectory(duration)
add_subdirectory(tests)
add_subdirectory(bindings)