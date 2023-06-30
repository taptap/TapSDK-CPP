# Build
## Clion
任何平台直接打开根目录即可
## Android Studio
打开 android 目录
## Build for ios
brew install binutils
export PATH="/opt/homebrew/opt/binutils/bin:$PATH"
run script/build-ios.sh
## XCode
run script/gen-xcode.sh 
open ios/build/tapsdk.xcodeproj
## VS Studio
使用 CMake UI 生成 .sln 工程
# Binding
## Java
-DJAVA_BINDING=${生成 Java 的目标路径}
## C#
-DCSHARP_BINDING=${生成 C# 的目标路径}
