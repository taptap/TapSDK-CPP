add_library(core STATIC
        runtime.cpp
        runtime.h
        events.h
)

target_link_libraries(core PRIVATE tapsdk::base)