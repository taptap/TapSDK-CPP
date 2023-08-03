set(SWIG_EXECUTABLE swig)
set(SWIG_SOURCE_FILE_EXTENSIONS .swg)

function(add_swig_jni_library TgtName)
  set(opts "")
  set(oneval_args GEN_JAVA_FILES_LIST)
  set(multival_args SOURCES)
  cmake_parse_arguments(SWIG_JNI_LIB
    "${opts}"
    "${oneval_args}"
    "${multival_args}"
    ${ARGN}
  )

  set_property(SOURCE
    ${SWIG_JNI_LIB_SOURCES}
    PROPERTY
      CPLUSPLUS On
  )

  set(PathInProject "bindings/java")
  string(REPLACE "/" "." JVMPackageName ${PathInProject})
  string(REPLACE "-" "_" JVMPackageName ${JVMPackageName})
  string(PREPEND JVMPackageName "com.taptap.tapsdk.")

  string(REPLACE "." "/" OutDir ${JVMPackageName})
  string(CONCAT OutDirAbs ${JAVA_BINDING}/${OutDir})

  swig_add_library(${TgtName}
    TYPE SHARED
    LANGUAGE java
    OUTPUT_DIR ${OutDirAbs}
    OUTFILE_DIR ${CMAKE_CURRENT_BINARY_DIR}/cpp
    SOURCES 
      ${SWIG_JNI_LIB_SOURCES}
  )

  if(APPLE)
    # for some legacy reason swig_add_library uses '.jnilib' suffix which has been replaced with '.dylib' since JDK7
    set_target_properties(${TgtName} PROPERTIES SUFFIX ".dylib")
  endif()

  set_property(TARGET ${TgtName} PROPERTY SWIG_COMPILE_OPTIONS -package ${JVMPackageName})

  add_custom_command(TARGET
    ${TgtName}
    POST_BUILD COMMAND
      ${CMAKE_COMMAND} -DJAVA_SRC_DIR=${OutDirAbs} -DJAVA_LST=${CMAKE_CURRENT_BINARY_DIR}/swig_gen_java.lst -P ${CMAKE_CURRENT_SOURCE_DIR}/gather_swig_java.cmake
  )
endfunction()
