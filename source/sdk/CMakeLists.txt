add_library(sdk STATIC
        tapsdk.cpp
        tapsdk.h
        )

target_link_libraries(sdk PUBLIC net)

if (APPLE)
    include(ios.cmake)
endif()