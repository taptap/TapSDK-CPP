set(protobuf_VERSION 21.4)
# 禁止 test 加速编译
set(protobuf_BUILD_TESTS OFF CACHE INTERNAL "")
# 禁止编译 protoc 加速编译
set(protobuf_BUILD_PROTOC_BINARIES OFF CACHE INTERNAL "")
set(protobuf_INSTALL OFF CACHE INTERNAL "")
# protobuf src
import_remote("https://github.com/protocolbuffers/protobuf.git" TAG "v${protobuf_VERSION}" CMAKE "./" ALIAS protobuf)

cmake_host_system_information(RESULT PROTOC_OS_RESULT QUERY OS_NAME)
message("PROTOC_OS_RESULT: ${PROTOC_OS_RESULT}")

if (${PROTOC_OS_RESULT} STREQUAL "Linux")
    set(protoc_PATFORM linux-x86_64)
    set(protoc_BINARY protoc)
elseif (${PROTOC_OS_RESULT} STREQUAL "macOS" OR ${PROTOC_OS_RESULT} STREQUAL "Darwin")
    set(protoc_PATFORM osx-universal_binary)
    set(protoc_BINARY protoc)
elseif (${PROTOC_OS_RESULT} STREQUAL "Windows")
    set(protoc_PATFORM win64)
    set(protoc_BINARY protoc.exe)
else ()
    message(FATAL_ERROR "cmake host ${PROTOC_OS_RESULT} do not support!")
endif ()

# download protoc compiler
if (NOT EXISTS ${protobuf_BINARY_DIR}/${protoc_BINARY})
    message("protoc download begin ==================================")
    file(DOWNLOAD
            "https://github.com/protocolbuffers/protobuf/releases/download/v${protobuf_VERSION}/protoc-${protobuf_VERSION}-${protoc_PATFORM}.zip"
            "${CMAKE_CURRENT_BINARY_DIR}/protoc.zip"
            STATUS DOWNLOAD_PROTOC_RESULT
            SHOW_PROGRESS
    )
    message("protoc download end ===================================")

    string(FIND "${DOWNLOAD_PROTOC_RESULT}" "0" DOWNLOAD_PROTOC_RESULT_POSITION)
    if (NOT ${DOWNLOAD_PROTOC_RESULT_POSITION} EQUAL -1)
        if (${PROTOC_OS_RESULT} STREQUAL "Windows")
            execute_process(
                    COMMAND powershell -Command "Expand-Archive -Path ${CMAKE_CURRENT_BINARY_DIR}/sprotoc.zip -DestinationPath ${CMAKE_CURRENT_BINARY_DIR}"
                    RESULTS_VARIABLE UNZIP_RESULT
            )
            file(COPY ${CMAKE_CURRENT_BINARY_DIR}/bin/${protoc_BINARY} DESTINATION ${protobuf_BINARY_DIR})
        else ()
            execute_process(
                    COMMAND unzip -n ${CMAKE_CURRENT_BINARY_DIR}/protoc.zip
                    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
                    RESULTS_VARIABLE UNZIP_RESULT
            )
            file(COPY ${CMAKE_CURRENT_BINARY_DIR}/bin/${protoc_BINARY} DESTINATION ${protobuf_BINARY_DIR})
            execute_process(
                    COMMAND chmod +x ${protobuf_BINARY_DIR}/${protoc_BINARY}
                    RESULTS_VARIABLE CHMOD_RESULT
            )
        endif ()
    endif ()
endif ()

