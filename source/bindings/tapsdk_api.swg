/* File : tapsdk_api.swg */
%module(directors="1") TapSDKNative
%{
/* Put headers and other declarations here */
#include "sdk/tapsdk_api.h"
%}

// 添加系统的一些文件，处理一些常用的基本类型，具体可以参考SWIG的帮助文档
%include stdint.i
%include carrays.i
%include windows.i
%include typemaps.i
#ifdef SWIGJAVA   //swig到java的swig预定义宏
%include <enums.swg>
%rename (toDString) toString;
#endif
// 生成 c 接口中的类型
%include "sdk/tapsdk_api.h"