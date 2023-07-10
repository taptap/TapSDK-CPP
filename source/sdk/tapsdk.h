#pragma once

#include <memory>
#include <string>
#include <cstdint>

namespace tapsdk {

class TDSUser;
using TDSUserHandle = const std::shared_ptr<TDSUser>&;

class TDSUser {
public:
    static void SetCurrent(TDSUserHandle user);
    static TDSUserHandle GetCurrent();

    virtual std::string GetUserId();
    virtual std::string GetUserName();
private:
    std::string user_id;
    std::string user_name;
};

class LoginCallback {
public:
    virtual void OnSuccess(TDSUserHandle user) = 0;
    virtual void OnFailed(int err_code, const char* msg) = 0;
};

bool Init();

void Login(const char* account, const char* passwd, LoginCallback *cb);

}  // namespace tapsdk
