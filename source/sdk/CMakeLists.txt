#export module

if (ANDROID OR WIN32 OR (CMAKE_SYSTEM_NAME STREQUAL "Linux"))
    set(SDK_BUILD_TYPE SHARED)
    set(TDS_LINK_FLAGS "-Wl,--exclude-libs,ALL")
else ()
    set(SDK_BUILD_TYPE STATIC)
    set(TDS_LINK_FLAGS)
endif()

add_library(tds_core
        ${SDK_BUILD_TYPE}
        tapsdk.cpp
        tapsdk.h
        platform.cpp
        platform.h
        )

target_link_libraries(tds_core PRIVATE duration net ${TDS_LINK_FLAGS})

set(PUBLIC_HEADER_FILES
        ${CMAKE_CURRENT_SOURCE_DIR}/tapsdk.h
        ${CMAKE_CURRENT_SOURCE_DIR}/platform.h
        ${CMAKE_CURRENT_SOURCE_DIR}/future.h
)

if (APPLE)
    include(apple.cmake)
endif()