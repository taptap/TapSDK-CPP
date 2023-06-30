add_library(sdk STATIC
        tapsdk.cpp
        tapsdk.h
        )

target_link_libraries(sdk PRIVATE base nlohmann_json httplib)

if (APPLE)
    include(ios.cmake)
endif()