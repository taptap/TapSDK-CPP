add_library(net STATIC
        network.cpp
        network.h
        httpclient.cpp
        httpclient.h
        coro_httpclient.cpp
        coro_httpclient.h
)

target_link_libraries(net PUBLIC tapsdk::base nlohmann_json)

if (WIN32)
    target_link_libraries(net PUBLIC wsock32)
    if (MINGW)
        target_link_libraries(net PUBLIC $<$<PLATFORM_ID:Windows>:ws2_32>)
    endif()
endif()

target_link_libraries(net PUBLIC ssl)

add_compile_definitions(CINATRA_ENABLE_SSL)
target_compile_definitions(net PRIVATE ASYNC_SIMPLE_HAS_NOT_AIO)
target_include_directories(net PUBLIC
        ${cinatra_SOURCE_DIR}/thirdparty/asio
        ${cinatra_SOURCE_DIR}/thirdparty/async_simple
        ${cinatra_SOURCE_DIR}/include
)