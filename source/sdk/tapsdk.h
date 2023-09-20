#pragma once

#include <cstdint>
#include <memory>
#include <string>
#include "result.h"

namespace tapsdk {

struct AccessToken {
    std::string kid;
    std::string token;
    std::string token_type;
    std::string mac_key;
    std::string mac_algorithm;
    uint64_t scope;
};

class TDSUser;

enum class Region : int { CN = 0, Global };

class TDSUser {
public:
    static void SetCurrent(const std::shared_ptr<TDSUser>& user);
    static std::shared_ptr<TDSUser> GetCurrent();

    TDSUser(const std::string& user_id = {});

    virtual ~TDSUser() = default;

    virtual std::string GetUserId();
    virtual std::string GetUserName() = 0;
    virtual bool ContainTapInfo() = 0;

private:
    std::string user_id;
};

class Game {
public:
    static void SetCurrent(const std::shared_ptr<Game>& game);
    static std::shared_ptr<Game> GetCurrent();

    virtual ~Game() = default;

    // 游戏 Game ID
    virtual std::string GetGameID() = 0;

    // 游戏包名/Bundle ID
    virtual std::string GetPackageName() = 0;
};

class TrackMessage {
public:
    explicit TrackMessage(std::string topic);

    virtual void AddContent(const std::string &key, const std::string &value) = 0;
    virtual void AddParam(const std::string &key, const std::string &value) = 0;

    std::string &GetTopic();

protected:
    std::string topic;
};

struct TrackerConfig {
    std::string endpoint;
    std::string access_keyid;
    std::string access_key_secret;
    std::string project;
    std::string log_store;
    int sdk_version;
    std::string sdk_version_name;
};

struct Config {
    bool enable_tap_login = false;
    bool enable_duration_statistics = false;
    bool enable_tap_tracker = false;
    std::string client_id{};
    Region region = Region::CN;
    TrackerConfig *tracker_config{};
};

// init
const char* SDKVersionName();
bool Init(const Config& config);

// login
Future<AccessToken> Login(const std::vector<std::string> &perm);

// tracker
std::shared_ptr<TrackMessage> CreateTracker(const std::string &topic);
void FlushTracker(const std::shared_ptr<TrackMessage> &tracker);

}  // namespace tapsdk
