set(protobuf_VERSION 24.3)
set(protobuf_BUILD_TESTS OFF)
set(protobuf_BUILD_PROTOC_BINARIES OFF)
# protobuf src
import_remote("https://github.com/protocolbuffers/protobuf.git" TAG "v${protobuf_VERSION}" CMAKE "./" ALIAS protobuf)

if (CMAKE_SYSTEM_NAME STREQUAL "Linux")
    set(protoc_PATFORM linux-x86_64)
    set(protoc_BINARY protoc)
elseif (CMAKE_SYSTEM_NAME STREQUAL "Darwin")
    set(protoc_PATFORM osx-universal_binary)
    set(protoc_BINARY protoc)
elseif (CMAKE_SYSTEM_NAME STREQUAL "Windows")
    set(protoc_PATFORM win64)
    set(protoc_BINARY protoc)
else ()
    message(FATAL_ERROR "cmake host ${CMAKE_SYSTEM_NAME} do not support!")
endif ()

# download protobuf bin
if (NOT EXISTS ${protobuf_BINARY_DIR}/${protoc_BINARY})
    file(DOWNLOAD
            "https://github.com/protocolbuffers/protobuf/releases/download/v${protobuf_VERSION}/protoc-${protobuf_VERSION}-${protoc_PATFORM}.zip"
            "${CMAKE_CURRENT_BINARY_DIR}/protoc.zip"
            STATUS DOWNLOAD_PROTOC_RESULT
            SHOW_PROGRESS
    )

    string(FIND "${DOWNLOAD_PROTOC_RESULT}" "0" DOWNLOAD_PROTOC_RESULT_POSITION)
    if (NOT ${DOWNLOAD_PROTOC_RESULT_POSITION} EQUAL -1)
        execute_process(
                COMMAND unzip -n ${CMAKE_CURRENT_BINARY_DIR}/protoc.zip
                WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
                RESULTS_VARIABLE UNZIP_RESULT
        )
        execute_process(
                COMMAND cp ${CMAKE_CURRENT_BINARY_DIR}/bin/${protoc_BINARY} ${protobuf_BINARY_DIR}/${protoc_BINARY}
                RESULTS_VARIABLE CP_RESULT
        )
        execute_process(
                COMMAND chmod +x ${protobuf_BINARY_DIR}/${protoc_BINARY}
                RESULTS_VARIABLE CHMOD_RESULT
        )
    endif ()
endif ()

