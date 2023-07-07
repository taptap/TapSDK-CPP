add_library(platform STATIC
        platform.cpp
        platform.h
        )

target_link_libraries(platform PUBLIC base)