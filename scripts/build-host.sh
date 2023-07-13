cd ../ || exit
mkdir build
cd build || exit
cmake ../ -G Ninja -DCMAKE_BUILD_TYPE=Release
ninja sdk -j8