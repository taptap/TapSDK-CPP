add_library(net STATIC
        network.cpp
        network.h
        httpclient.cpp
        httpclient.h
        coro_httpclient.cpp
        coro_httpclient.h
)

target_link_libraries(net PUBLIC base nlohmann_json httplib)

find_package(OpenSSL 1.1)
if (NOT OPENSSL_FOUND)
    target_link_libraries(net PUBLIC ssl crypto)
endif()

add_compile_definitions(CINATRA_ENABLE_SSL)
target_compile_definitions(net PRIVATE ASYNC_SIMPLE_HAS_NOT_AIO)
target_include_directories(net PRIVATE
        ${cinatra_SOURCE_DIR}/thirdparty/asio
        ${cinatra_SOURCE_DIR}/thirdparty/async_simple
        ${cinatra_SOURCE_DIR}/include
)