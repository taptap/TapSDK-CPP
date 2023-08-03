set(CMAKE_C_VISIBILITY_PRESET default)
set(CMAKE_CXX_VISIBILITY_PRESET default)
add_library(platform STATIC
        platform.cpp
        )

target_link_libraries(platform PRIVATE base)