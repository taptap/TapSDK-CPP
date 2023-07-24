cd ../ios || exit
mkdir build
cd build || exit
cmake ../../ -G Xcode -DCMAKE_TOOLCHAIN_FILE=../ios.toolchain.cmake  -DCMAKE_BUILD_TYPE=Debug  -DDEPLOYMENT_TARGET=14.0 -DCMAKE_OSX_ARCHITECTURES="arm64 | x86_64"  -DENABLE_BITCODE=0 -DPLATFORM=OS64COMBINED -DCSHARP_BINDING=./swig