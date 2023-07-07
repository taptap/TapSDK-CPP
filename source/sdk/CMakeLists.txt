add_library(sdk SHARED
        tapsdk.cpp
        tapsdk.h
        )

target_link_libraries(sdk PUBLIC duration net platform)

if (APPLE)
    include(ios.cmake)
endif()