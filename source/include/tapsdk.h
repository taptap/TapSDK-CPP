#pragma once

#include <memory>
#include <string>
#include <cstdint>

namespace tapsdk {

class TDSUser {
public:
    int GetUserId();
    const char* GetUserName();

private:
    int user_id;
    std::string user_name;
};

class LoginCallback {
public:
    virtual void OnSuccess(const std::shared_ptr<TDSUser>& user) = 0;
    virtual void OnFailed(int err_code, const char* msg) = 0;
};

bool Init();

void Login(const char* account, const char* passwd, LoginCallback *cb);

}  // namespace tapsdk
