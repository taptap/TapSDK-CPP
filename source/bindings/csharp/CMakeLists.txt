# SWIG
# ======================================
find_package(SWIG REQUIRED)
include(${SWIG_USE_FILE})

set(CMAKE_SWIG_FLAGS "")

include_directories($ENV{JAVA_HOME}/include)

set_source_files_properties(example.i PROPERTIES CPLUSPLUS ON)
set_source_files_properties(example.i PROPERTIES SWIG_FLAGS "-includeall")
swig_add_library(example csharp example.i example.cxx)