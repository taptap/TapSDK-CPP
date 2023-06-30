add_subdirectory(fmt)
add_subdirectory(cpp-httplib)
add_subdirectory(nlohmann_json)
add_subdirectory(catch)
#add_subdirectory(marl)

# openssl
find_package(OpenSSL 1.1)
if (OPENSSL_FOUND)
    set(OPENSSL_LIBRARIES OpenSSL::SSL OpenSSL::Crypto)
else()
    add_subdirectory(openssl)
endif()