# TapSDK
TapSDK cpp (WIP)
# Build
## 依赖
brew install swig
## Clion
任何平台直接打开根目录即可
## Android Studio
打开 android 目录
## Build for android
run scripts/build-android.sh
## Build for ios
brew install binutils
export PATH="/opt/homebrew/opt/binutils/bin:$PATH"
run scripts/build-ios.sh
## XCode
run scripts/gen-xcode.sh 
open ios/build/tapsdk.xcodeproj
## VS Studio
使用 CMake UI 生成 .sln 工程
# Binding
## Java
-DJAVA_BINDING=${生成 Java 的目标路径}
## C#
-DCSHARP_BINDING=${生成 C# 的目标路径}
# 平台适配
## /source/sdk/platform.h
### Device
实现平台 Device 并 调用 Device.SetCurrent
- Device.OnBackground
当 App 进入后台时调用
- Device.OnForeground
当 App 进入前台时调用
### Window
实现平台 Window 并 调用 Window.SetCurrent
## /source/sdk/tapsdk.h
当用户登陆 / 登出时调用 TDSUser.SetCurrent

