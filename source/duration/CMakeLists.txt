add_library(duration STATIC
        duration.cpp
        duration.h
        persistence.cpp
        persistence.h
        dur_model.cpp
        dur_model.h
)

target_link_libraries(duration PUBLIC net core sqlite_orm)