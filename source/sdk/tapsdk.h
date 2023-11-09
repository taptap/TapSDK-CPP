#pragma once

#include <cstdint>
#include <memory>
#include <string>
#include <vector>
#include "result.h"
#include "config.h"

namespace tapsdk {

struct TDS_EXPORT AccessToken {
    std::string kid;
    std::string token;
    std::string token_type;
    std::string mac_key;
    std::string mac_algorithm;
    uint64_t scope;
};

class TDSUser;

enum class Region { CN = 0, Global, RND };

class TDS_EXPORT TDSUser {
public:
    static void SetCurrent(const std::shared_ptr<TDSUser>& user);
    static std::shared_ptr<TDSUser> GetCurrent();

    TDSUser(const std::string& user_id = "");

    virtual ~TDSUser() = default;

    virtual std::string GetUserId();
    virtual std::string GetUserName() = 0;
    virtual bool ContainTapInfo() = 0;

private:
    std::string user_id;
};

class TDS_EXPORT Game {
public:
    static void SetCurrent(const std::shared_ptr<Game>& game);
    static std::shared_ptr<Game> GetCurrent();

    virtual ~Game() = default;

    // 游戏 Game ID
    virtual std::string GetGameID() = 0;

    // 游戏包名/Bundle ID
    virtual std::string GetPackageName() = 0;
};

struct TDS_EXPORT TrackerConfig {
    std::string topic;
    std::string endpoint;
    std::string access_keyid;
    std::string access_key_secret;
    std::string project;
    std::string log_store;
    int sdk_version;
    std::string sdk_version_name;
    uint64_t hash = 0;

    uint64_t Hash();
};

class TDS_EXPORT TrackMessage {
public:
    explicit TrackMessage(const std::shared_ptr<TrackerConfig> &config);

    virtual void AddContent(const std::string &key, const std::string &value) = 0;
    virtual void AddParam(const std::string &key, const std::string &value) = 0;

    std::shared_ptr<TrackerConfig> GetConfig() const;
    uint32_t GetCreateTime() const;

protected:
    std::shared_ptr<TrackerConfig> config;
    uint32_t create_time;
};

struct TDS_EXPORT Config {
    bool enable_tap_login = false;
    bool enable_duration_statistics = false;
    bool enable_tap_tracker = false;
    std::string client_id = "";
    std::string process_name = "";
    Region region = Region::CN;
    std::string sdk_version = "";
    uint32_t tracker_group_size = 0;
};

// init
TDS_EXPORT const char* SDKVersionName();
TDS_EXPORT bool Init(const Config& config);

// login
TDS_EXPORT Future<AccessToken> Login(const std::vector<std::string> &perm);

// tracker
TDS_EXPORT std::shared_ptr<TrackMessage> CreateTracker(const std::shared_ptr<TrackerConfig> &config);
TDS_EXPORT bool FlushTracker(const std::shared_ptr<TrackMessage> &tracker);

}  // namespace tapsdk
