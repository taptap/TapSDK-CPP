add_library(duration STATIC
        duration.cpp
        duration.h
        persistence.cpp
        persistence.h
        model.cpp
        model.h
)

target_link_libraries(duration PUBLIC net core sqlite_orm)