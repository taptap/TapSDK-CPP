#export module

if (ANDROID)
    set(SDK_BUILD_TYPE SHARED)
else ()
    set(SDK_BUILD_TYPE STATIC)
endif()

add_library(sdk
        ${SDK_BUILD_TYPE}
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

if (IOS)
    include(ios.cmake)
endif()