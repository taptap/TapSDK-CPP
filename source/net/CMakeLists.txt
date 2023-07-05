add_library(net STATIC
        network.cpp
        network.h
        )

target_link_libraries(net PUBLIC base nlohmann_json httplib)

target_compile_definitions(net PRIVATE ASYNC_SIMPLE_HAS_NOT_AIO)
target_include_directories(net PRIVATE
        ${cinatra_SOURCE_DIR}/thirdparty/asio
        ${cinatra_SOURCE_DIR}/thirdparty/async_simple
        ${cinatra_SOURCE_DIR}/include
)