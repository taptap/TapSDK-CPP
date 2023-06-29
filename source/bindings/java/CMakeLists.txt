# SWIG
# ======================================
find_package(SWIG REQUIRED)
include(${SWIG_USE_FILE})
include(swig.cmake)

add_swig_jni_library(bindings-java GEN_JAVA_FILES_LIST ${CMAKE_CURRENT_BINARY_DIR}/swig_gen_java.lst SOURCES ${SWIG_DEFINES})
target_link_libraries(bindings-java PUBLIC android bindings-impl)
