cd ../ || exit
mkdir build
cd build || exit
cmake ../ -G Ninja -DCMAKE_BUILD_TYPE=Release -DCSHARP_BINDING=./swig
ninja sdk -j8
ninja bindings-csharp -j8