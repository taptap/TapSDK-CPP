add_library(bindings-impl STATIC
        bindings.cpp
        bindings.h)

target_link_libraries(bindings-impl PUBLIC tds_core)

# bindings for java / csharp
file(GLOB_RECURSE SWIG_DEFINES ${CMAKE_CURRENT_SOURCE_DIR}/*.swg)

# config swig
find_package(SWIG)
if (NOT SWIG_FOUND)
    cmake_host_system_information(RESULT SWIG_OS_RESULT QUERY OS_NAME)
    if (${SWIG_OS_RESULT} STREQUAL "Windows")
        set(SWIG_DOWNLOAD_DIR ${CMAKE_CURRENT_BINARY_DIR}/swig_home)
        set(SWIG_URL "https://nchc.dl.sourceforge.net/project/swig/swigwin/swigwin-4.1.1/swigwin-4.1.1.zip")
        set(SWIG_DIR ${SWIG_DOWNLOAD_DIR}/swigwin-4.1.1)
        set(SWIG_EXECUTABLE ${SWIG_DIR}/swig.exe)
        if (NOT EXISTS ${SWIG_DOWNLOAD_DIR})
            message("swig download begin ==================================")
            file(DOWNLOAD
                    ${SWIG_URL}
                    "${CMAKE_CURRENT_BINARY_DIR}/swig.zip"
                    STATUS DOWNLOAD_SWIG_RESULT
                    SHOW_PROGRESS
            )
            message("swig download end ===================================")
            string(FIND "${DOWNLOAD_SWIG_RESULT}" "0" DOWNLOAD_SWIG_RESULT_POSITION)
            if (NOT ${DOWNLOAD_SWIG_RESULT_POSITION} EQUAL -1)
                execute_process(
                        COMMAND powershell -Command "Expand-Archive -Path ${CMAKE_CURRENT_BINARY_DIR}/swig.zip -DestinationPath ${SWIG_DOWNLOAD_DIR}"
                        RESULTS_VARIABLE UNZIP_RESULT
                )
            endif ()
        endif ()
    elseif (${SWIG_OS_RESULT} STREQUAL "macOS" OR ${PROTOC_OS_RESULT} STREQUAL "Darwin")
        execute_process(
                COMMAND brew install swig
        )
    elseif (${SWIG_OS_RESULT} STREQUAL "Linux")
        execute_process(
                COMMAND sudo apt-get install swig
        )
    else ()
        message(FATAL_ERROR "swig not config but cmake host ${SWIG_OS_RESULT} do not support!")
    endif ()
endif ()

if (DEFINED JAVA_BINDING)
    add_subdirectory(java)
endif()

if (DEFINED CSHARP_BINDING)
    add_subdirectory(csharp)
endif()