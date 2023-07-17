cd ../ios || exit
mkdir build
cd build || exit
cmake ../../ -G Ninja -DCMAKE_TOOLCHAIN_FILE=../ios.toolchain.cmake -DCMAKE_BUILD_TYPE=Release -DDEPLOYMENT_TARGET=14.0 -DCMAKE_OSX_ARCHITECTURES="arm64" -DENABLE_BITCODE=0 -DPLATFORM=OS64
ninja sdk_combine -j8