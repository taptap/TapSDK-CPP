# Find required protobuf package
set(Protobuf_USE_STATIC_LIBS ON)
set(Protobuf_INCLUDE_DIR ${protobuf_SOURCE_DIR}/src)
set(Protobuf_LIBRARIES ${protobuf_BINARY_DIR})
set(PROTOBUF_LITE_LIBRARY libprotobuf-lite)

# Set protoc compiler
cmake_host_system_information(RESULT PROTOC_OS_RESULT QUERY OS_NAME)
if (${PROTOC_OS_RESULT} STREQUAL "Windows")
    set(PROTOC_EXECUTABLE ${protobuf_BINARY_DIR}/protoc.exe)
else ()
    set(PROTOC_EXECUTABLE ${protobuf_BINARY_DIR}/protoc)
endif ()
add_executable(protobuf::protoc IMPORTED)
set_property(TARGET protobuf::protoc PROPERTY IMPORTED_LOCATION "${PROTOC_EXECUTABLE}")

find_package(Protobuf REQUIRED)
if(PROTOBUF_FOUND)
    message(STATUS "protobuf library found")
else()
    message(FATAL_ERROR "protobuf library is needed but cant be found")
endif()

file(GLOB TRACKER_PROTO_FILES "proto/*.proto")
protobuf_generate_cpp(TRACKER_PROTO_SRCS TRACKER_PROTO_HDRS ${TRACKER_PROTO_FILES})

add_library(tracker STATIC
        tap_tracker.h
        model.cpp
        model.h
        tracker_impl.cpp
        tracker_impl.h
        ${TRACKER_PROTO_SRCS}
        cache.cpp
        cache.h)

target_link_libraries(tracker PUBLIC net core sqlite_orm ${PROTOBUF_LITE_LIBRARY} lz4_static)
target_include_directories(tracker PRIVATE ${PROTOBUF_INCLUDE_DIRS} ${TRACKER_PROTO_HDRS})
target_include_directories(tracker PRIVATE ${CMAKE_CURRENT_BINARY_DIR})