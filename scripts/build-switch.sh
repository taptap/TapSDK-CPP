cd ../switch || exit
mkdir build
cd build || exit
cmake ../../ -G Ninja -DCMAKE_TOOLCHAIN_FILE=../toolchain-switch.cmake -DCMAKE_BUILD_TYPE=Release
ninja tds_core -j8