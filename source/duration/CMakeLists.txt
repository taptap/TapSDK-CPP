add_library(duration STATIC
        duration.cpp
        duration.h
        )

target_link_libraries(duration PUBLIC net)