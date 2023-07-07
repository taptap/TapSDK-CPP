add_library(platform STATIC
        platform.cpp
        )

target_link_libraries(platform PRIVATE base)