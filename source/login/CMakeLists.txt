add_library(login STATIC
        tap_login.cpp
        tap_login.h
        model.cpp
        model.h
)

target_link_libraries(login PUBLIC net core sqlite_orm)