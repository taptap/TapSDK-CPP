//
// Created by 甘尧 on 2023/9/12.
//

#include "base/types.h"
#include "sdk/result.h"
#include "sdk/tapsdk.h"

namespace tapsdk::login {

Result<> Init(const Config &config);
Future<AccessToken> Login(const std::vector<std::string> &perm);

}
