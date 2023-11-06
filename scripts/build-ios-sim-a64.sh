cd ../ios || exit
mkdir build-sim-a64
cd build-sim-a64 || exit
cmake ../../ -G Ninja -DCMAKE_TOOLCHAIN_FILE=../ios.toolchain.cmake -DCMAKE_BUILD_TYPE=Release -DDEPLOYMENT_TARGET=11.0 -DCMAKE_OSX_ARCHITECTURES="arm64" -DENABLE_BITCODE=0 -DPLATFORM=SIMULATORARM64 -DCSHARP_BINDING=./swig
ninja sdk_combine -j8