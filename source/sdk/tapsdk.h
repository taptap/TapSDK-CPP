#pragma once

#include <memory>
#include <string>
#include <cstdint>

namespace tapsdk {

class TDSUser;

class TDSUser {
public:
    static void SetCurrent(const std::shared_ptr<TDSUser>& user);
    static std::shared_ptr<TDSUser> GetCurrent();

    virtual std::string GetUserId();
    virtual std::string GetUserName();
private:
    std::string user_id;
    std::string user_name;
};

class Game {
public:
    static void SetCurrent(const std::shared_ptr<Game> &game);
    static std::shared_ptr<Game> GetCurrent();

    Game(const std::string &uuid);

    // 游戏唯一标志
    virtual std::string GetUUID();

    // 游戏 Game ID (可不实现)
    virtual std::uint64_t GetGameID();

    // 游戏包名/Bundle ID (可不实现)
    virtual std::string GetPackageName();
private:
    std::string uuid;
};

struct Config {
    bool enable_duration_statistics;
};

bool Init(const Config &config);

}  // namespace tapsdk
