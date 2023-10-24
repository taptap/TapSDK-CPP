cd ../ios || exit
mkdir build-tvos-sim
cd build-tvos-sim || exit
cmake ../../ -G Ninja -DCMAKE_TOOLCHAIN_FILE=../ios.toolchain.cmake -DCMAKE_BUILD_TYPE=Release -DDEPLOYMENT_TARGET=11.0 -DCMAKE_OSX_ARCHITECTURES="x86_64" -DENABLE_BITCODE=0 -DPLATFORM=SIMULATOR_TVOS -DCSHARP_BINDING=./swig
ninja sdk_combine -j8