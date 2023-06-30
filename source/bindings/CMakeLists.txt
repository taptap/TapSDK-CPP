add_library(bindings-impl STATIC
        bindings.cpp
        bindings.h)

target_link_libraries(bindings-impl PUBLIC sdk)

# bindings for java / csharp
file(GLOB_RECURSE SWIG_DEFINES ${CMAKE_CURRENT_SOURCE_DIR}/*.swg)

if (DEFINED JAVA_BINDING)
    add_subdirectory(java)
endif()

if (DEFINED CSHARP_BINDING)
    add_subdirectory(csharp)
endif()