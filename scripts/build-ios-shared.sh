cd ../ios || exit
mkdir build
cd build || exit
cmake ../../ -G Xcode -DCMAKE_TOOLCHAIN_FILE=../ios.toolchain.cmake -DCMAKE_BUILD_TYPE=Release -DDEPLOYMENT_TARGET=11.0 -DCMAKE_OSX_ARCHITECTURES="arm64 | x86_64" -DENABLE_BITCODE=0 -DPLATFORM=OS64COMBINED -DCSHARP_BINDING=./swig -DBUILD_FOR_SHARED=1
xcodebuild build -scheme tds_core -sdk iphoneos -configuration Release OTHER_CFLAGS="-Wl,-ld_classic"
xcodebuild build -scheme bindings-csharp -sdk iphoneos -configuration Release OTHER_CFLAGS="-Wl,-ld_classic"
xcodebuild build -scheme tds_core -sdk iphonesimulator -configuration Release OTHER_CFLAGS="-Wl,-ld_classic"
xcodebuild build -scheme bindings-csharp -sdk iphonesimulator -configuration Release OTHER_CFLAGS="-Wl,-ld_classic"