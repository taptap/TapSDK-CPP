add_library(login STATIC
        tap_login.cpp
        tap_login.h
        login_model.cpp
        login_model.h
)

target_link_libraries(login PUBLIC net core sqlite_orm)