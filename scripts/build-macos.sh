cd ../ || exit
mkdir build-macos
cd build-macos || exit
cmake ../ -G Ninja -DCMAKE_TOOLCHAIN_FILE=../ios/ios.toolchain.cmake -DCMAKE_BUILD_TYPE=Release -DDEPLOYMENT_TARGET=11.0 -DENABLE_BITCODE=0 -DPLATFORM=MAC_UNIVERSAL -DCSHARP_BINDING=./swig
ninja sdk_combine -j8