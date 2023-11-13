#export module

if (ANDROID OR (CMAKE_SYSTEM_NAME STREQUAL "Linux"))
    set(SDK_BUILD_TYPE SHARED)
    set(TDS_LINK_FLAGS "-Wl,--exclude-libs,ALL")
else ()
    set(SDK_BUILD_TYPE STATIC)
    set(TDS_LINK_FLAGS)
endif()

if (DEFINED BUILD_FOR_SHARED)
    set(SDK_BUILD_TYPE SHARED)
endif ()

add_library(tds_core
        ${SDK_BUILD_TYPE}
        tapsdk.cpp
        tapsdk.h
        platform.cpp
        platform.h
        result.h
        tapsdk_api.cpp
        tapsdk_api.h
)

target_link_libraries(tds_core PRIVATE duration login tracker net ${TDS_LINK_FLAGS})

if (WIN32)
    # Tell CMake to locate the lib.exe tool.
    get_filename_component(VS_BIN_DIR "${CMAKE_LINKER}" DIRECTORY)
    message("VS_BIN_DIR: ${VS_BIN_DIR}")
    find_program(MSVC_LIB_TOOL ${VS_BIN_DIR}/lib.exe)

    # If the tool was found, create the custom target.
    if(MSVC_LIB_TOOL)
        add_custom_target(tds_core_combined
                ALL
                COMMAND ${MSVC_LIB_TOOL} /OUT:$<TARGET_FILE_DIR:tds_core>\\tds_core_combined.lib
                $<TARGET_FILE:tds_core>
                $<TARGET_FILE:tapsdk::base>
                $<TARGET_FILE:fmt>
                $<TARGET_FILE:net>
                $<TARGET_FILE:core>
                $<TARGET_FILE:duration>
                $<TARGET_FILE:login>
                $<TARGET_FILE:tracker>
                $<TARGET_FILE:EventBus>
                $<TARGET_FILE:ssl>
                $<TARGET_FILE:crypto>
                $<TARGET_FILE:sqlite3>
                $<TARGET_FILE:libprotobuf-lite>
                $<TARGET_FILE:lz4_static>
                $<TARGET_FILE:bindings-impl>
                DEPENDS tds_core net tapsdk::base core bindings-impl duration login tracker
                WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
                )
    endif()
endif ()

set(PUBLIC_HEADER_FILES
        ${CMAKE_CURRENT_SOURCE_DIR}/tapsdk.h
        ${CMAKE_CURRENT_SOURCE_DIR}/platform.h
        ${CMAKE_CURRENT_SOURCE_DIR}/result.h
        ${CMAKE_CURRENT_SOURCE_DIR}/tapsdk_api.h
)

if (APPLE)
    include(apple.cmake)
endif()