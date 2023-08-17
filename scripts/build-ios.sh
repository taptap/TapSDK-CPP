cd ../ios || exit
mkdir build
cd build || exit
cmake ../../ -G Ninja -DCMAKE_TOOLCHAIN_FILE=../ios.toolchain.cmake -DCMAKE_BUILD_TYPE=Release -DDEPLOYMENT_TARGET=12.0 -DCMAKE_OSX_ARCHITECTURES="arm64" -DENABLE_BITCODE=0 -DPLATFORM=OS64 -DCSHARP_BINDING=./swig
ninja sdk_combine -j8