if(_SWITCH_TOOLCHAIN_)
	return()
endif()
set(_SWITCH_TOOLCHAIN_ 1)

set(NX true)
set(NINTENDO_SWITCH true)

# Add our custom props file, which lets us set key properties the NX prop files are expecting 
file(GENERATE OUTPUT SwitchBuild.props INPUT ${CMAKE_CURRENT_LIST_DIR}/SwitchBuild.props)

function(add_executable target_name)
  _add_executable(${target_name} ${ARGN})
  set_target_properties(${target_name} PROPERTIES VS_USER_PROPS ${CMAKE_CURRENT_LIST_DIR}/SwitchBuild.props)
endfunction()

function(add_library target_name)
  _add_library(${target_name} ${ARGN})
  set_target_properties(${target_name} PROPERTIES VS_USER_PROPS ${CMAKE_CURRENT_LIST_DIR}/SwitchBuild.props)
endfunction()

# TODO: This was just so CMake didn't fail when identifying the compiler (as it doesn't create nnMain and the example app doesn't link) but doesn't seem to change anything...
set(CMAKE_TRY_COMPILE_TARGET_TYPE STATIC_LIBRARY)

# Manual identification until we can fix CMake's detection
set(SDK_ROOT "$ENV{NINTENDO_SDK_ROOT}")
string(REPLACE "\\" "/" SDK_ROOT ${SDK_ROOT})
set(CMAKE_C_COMPILER "${SDK_ROOT}/Compilers/NX/nx/aarch64/bin/clang.exe")
set(CMAKE_CXX_COMPILER "${SDK_ROOT}/Compilers/NX/nx/aarch64/bin/clang++.exe")
# I don't think this actually does anything, but we need a way to fix CMake not being able to identify the compiler
set(CMAKE_C_COMPILER_ID Clang)
set(CMAKE_CXX_COMPILER_ID Clang)

# Tell CMake we are cross-compiling to NX64
set(CMAKE_SYSTEM_NAME "Generic")
set(CMAKE_GENERATOR_PLATFORM NX64)

# Without this, MSVC won't inherit the dependencies and we'll fail to add the NX base libraries
set(CMAKE_CXX_STANDARD_LIBRARIES "%(AdditionalDependencies)")
