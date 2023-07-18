# SWIG
# ======================================
find_package(SWIG REQUIRED)
include(${SWIG_USE_FILE})
include(swig.cmake)

add_swig_csharp_library(bindings-csharp GEN_CSHARP_FILES_LIST ${CMAKE_CURRENT_BINARY_DIR}/swig_gen_csharp.lst SOURCES ${SWIG_DEFINES})
target_link_libraries(bindings-csharp PUBLIC bindings-impl)
