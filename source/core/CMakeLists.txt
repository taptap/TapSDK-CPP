add_library(core STATIC
        runtime.cpp
        runtime.h
)

target_link_libraries(core PRIVATE base)