set(SWIG_EXECUTABLE swig)
set(SWIG_SOURCE_FILE_EXTENSIONS .swg)

function(add_swig_csharp_library TgtName)
  set(opts "")
  set(oneval_args GEN_CSHARP_FILES_LIST)
  set(multival_args SOURCES)
  cmake_parse_arguments(SWIG_CSHARP_LIB
    "${opts}"
    "${oneval_args}"
    "${multival_args}"
    ${ARGN}
  )

  set_property(SOURCE
    ${SWIG_CSHARP_LIB_SOURCES}
    PROPERTY
      CPLUSPLUS On
  )

  set(PathInProject "bindings/csharp")
  string(REPLACE "/" "." CSharpPackageName ${PathInProject})
  string(REPLACE "-" "_" CSharpPackageName ${CSharpPackageName})
  string(PREPEND CSharpPackageName "com.taptap.tapsdk.")

  string(REPLACE "." "/" OutDir ${CSharpPackageName})
  string(CONCAT OutDirAbs ${CSHARP_BINDING}/${OutDir})

  if (APPLE)
    swig_add_library(${TgtName}
            TYPE STATIC
            LANGUAGE csharp
            OUTPUT_DIR ${OutDirAbs}
            OUTFILE_DIR ${CMAKE_CURRENT_BINARY_DIR}/cpp
            SOURCES
            ${SWIG_CSHARP_LIB_SOURCES}
    )
    set_target_properties(${TgtName} PROPERTIES SUFFIX ".a")
  else ()
    swig_add_library(${TgtName}
            TYPE SHARED
            LANGUAGE csharp
            OUTPUT_DIR ${OutDirAbs}
            OUTFILE_DIR ${CMAKE_CURRENT_BINARY_DIR}/cpp
            SOURCES
            ${SWIG_CSHARP_LIB_SOURCES}
    )
  endif()

  set_property(TARGET ${TgtName} PROPERTY SWIG_COMPILE_OPTIONS -namespace ${CSharpPackageName})

  add_custom_command(TARGET
    ${TgtName}
    POST_BUILD COMMAND
      ${CMAKE_COMMAND} -DCSHARP_SRC_DIR=${OutDirAbs} -DCSHARP_LST=${CMAKE_CURRENT_BINARY_DIR}/swig_gen_csharp.lst -P ${CMAKE_CURRENT_SOURCE_DIR}/gather_swig_csharp.cmake
  )
endfunction()
