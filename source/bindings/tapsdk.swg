/* File : tapsdk.swg */
%module(directors="1") TapSDK
%{
/* Put headers and other declarations here */
#include "bindings/bindings.h"
#include <memory>
#include <vector>
using namespace tapsdk;
%}

// 添加系统的一些文件，处理一些常用的基本类型，具体可以参考SWIG的帮助文档
%include stdint.i
%include std_string.i
%include std_vector.i
%include carrays.i
%include windows.i
%include typemaps.i
%include std_shared_ptr.i
#ifdef SWIGJAVA   //swig到java的swig预定义宏
%include <enums.swg>
%rename (toDString) toString;
#endif
// 生成 cpp 接口中的类型
%shared_ptr(tapsdk::TDSUser);
%shared_ptr(tapsdk::Game);
%shared_ptr(tapsdk::TrackerConfig);
%shared_ptr(tapsdk::TrackMessage);
%shared_ptr(tapsdk::AccessToken);
%feature("director") TDSUser;
%feature("director") Game;
%feature("director") FutureCallback<AccessToken>;
%include "sdk/result.h"
%include "sdk/tapsdk.h"
namespace tapsdk {
%template(TDSTokenCallback) FutureCallback<AccessToken>;
%template(TDSTokenResult) Result<AccessToken>;
%template(TDSTokenFuture) Future<AccessToken>;
}
%template(StringVector) std::vector<std::string>;