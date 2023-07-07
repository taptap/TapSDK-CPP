#export module

add_library(sdk SHARED
        tapsdk.cpp
        tapsdk.h
        platform.h
        )

target_link_libraries(sdk PUBLIC platform)
target_link_libraries(sdk PRIVATE duration net)

set(PUBLIC_HEADER_FILES
        ${CMAKE_CURRENT_SOURCE_DIR}/tapsdk.h
        ${CMAKE_CURRENT_SOURCE_DIR}/platform.h
)

if (APPLE)
    include(ios.cmake)
endif()