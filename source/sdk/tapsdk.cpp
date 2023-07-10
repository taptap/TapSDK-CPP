//
// Created by 甘尧 on 2023/6/29.
//

#include <unistd.h>
#include "base/logging.h"
#include "core/runtime.h"
#include "net/network.h"
#include "tapsdk.h"

namespace tapsdk {

std::shared_ptr<TDSUser> current_user;

void TDSUser::SetCurrent(TDSUserHandle user) {
    LOG_ERROR("UserName {}", user->GetUserName());
    current_user = user;
}

TDSUserHandle TDSUser::GetCurrent() {
    return current_user;
}

std::string TDSUser::GetUserId() const {
    return user_id;
}

std::string TDSUser::GetUserName() const {
    return user_name;
}

bool Init() {
    Runtime::Get().Init();
    return true;
}

void Login(const char* account, const char* passwd, LoginCallback *cb) {
    cb->OnFailed(-1, "Unimplemented");
}

}
