add_subdirectory(fmt)
add_subdirectory(cpp-httplib)
add_subdirectory(nlohmann_json)
add_subdirectory(catch)
add_subdirectory(cinatra)
add_subdirectory(sqlite3)
add_subdirectory(sqlite_orm)
add_subdirectory(event_bus)
#add_subdirectory(marl)

# openssl
find_package(OpenSSL 1.1)
if (OPENSSL_FOUND)
    set(OPENSSL_LIBRARIES OpenSSL::SSL OpenSSL::Crypto)
else()
    add_subdirectory(openssl)
endif()

# ranges
if (ANDROID)
    add_library(compat INTERFACE)
    target_include_directories(compat INTERFACE compat/ranges)
    target_include_directories(compat INTERFACE compat)
endif ()