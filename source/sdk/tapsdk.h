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

struct Config {
    bool enable_duration_statistics;
};

bool Init(const Config &config);

}  // namespace tapsdk
