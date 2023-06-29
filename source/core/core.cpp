//
// Created by 甘尧 on 2023/6/29.
//

#include "core.h"

namespace tapsdk {

int TDSUser::GetUserId() {
    return user_id;
}

const char *TDSUser::GetUserName() {
    return user_name.c_str();
}

bool Init() {
    return false;
}

void Login(const char* account, const char* passwd, LoginCallback *cb) {
    cb->OnFailed(-1, "Unimplemented");
}

}
