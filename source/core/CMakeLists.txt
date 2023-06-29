add_library(core STATIC
        core.cpp
        core.h)

target_link_libraries(core PUBLIC base)