cd ../ || exit
mkdir build-a64
cd build-a64 || exit
cmake ../ -G Ninja -DCMAKE_BUILD_TYPE=Release -DENABLE_ARC=0 -DENABLE_BITCODE=0 -DCMAKE_OSX_ARCHITECTURES="arm64" -DCSHARP_BINDING=./swig
ninja sdk_combine -j8