//
// Created by 甘尧 on 2023/6/29.
//

#include <atomic>
#include "base/logging.h"
#include "base/cityhash.h"
#include "core/events.h"
#include "core/runtime.h"
#include "duration/duration.h"
#include "login/tap_login.h"
#include "tapsdk.h"
#include "tracker/tap_tracker.h"

namespace tapsdk {

static Config sdk_config;

// users
static std::mutex user_lock;
static std::shared_ptr<TDSUser> current_user;

static std::shared_ptr<Game> current_game;

// modules
static std::unique_ptr<duration::DurationStatistics> duration_statistics;
static std::atomic<bool> inited{false};

const char* SDKVersionName() { return TDS_VERSION; }

bool Init(const Config& config) {
    sdk_config = config;
    try {
        ASSERT_MSG(!inited, "SDK already inited!");
        inited = true;
        Runtime::Get().Init();
        if (!platform::Device::GetCurrent()) {
            LOG_ERROR("need device!");
            return false;
        }
        if (sdk_config.enable_duration_statistics) {
            duration_statistics = std::make_unique<duration::DurationStatistics>();
            duration_statistics->Init(sdk_config);
        }
        if (sdk_config.enable_tap_login) {
            login::Init(config);
        }
        if (sdk_config.enable_tap_tracker) {
            tracker::Init(config);
        }
        return true;
    } catch (...) {
        return false;
    }
}

void TDSUser::SetCurrent(const std::shared_ptr<TDSUser>& user) {
    std::scoped_lock guard(user_lock);
    current_user = user;
    Runtime::Get().GetEventBus()->notifyNow(events::User{user});
}

std::shared_ptr<TDSUser> TDSUser::GetCurrent() { return current_user; }

std::string TDSUser::GetUserId() { return user_id; }

TDSUser::TDSUser(const std::string& id) : user_id{id} {}

void Game::SetCurrent(const std::shared_ptr<Game>& game) {
    current_game = game;
    Runtime::Get().GetEventBus()->notifyNow(events::GameStart{game});
}

std::shared_ptr<Game> Game::GetCurrent() { return current_game; }

Future<AccessToken> Login(const std::vector<std::string>& perm) { return login::Login(perm); }

TrackMessage::TrackMessage(const std::shared_ptr<TrackerConfig> &config) : config(config) {}

std::shared_ptr<TrackerConfig> TrackMessage::GetConfig() const {
    return config;
}

uint32_t TrackMessage::GetCreateTime() const { return create_time; }

uint64_t TrackerConfig::Hash() {
    if (hash) {
        return hash;
    }

    struct {
        u64 topic;
        u64 endpoint;
        u64 access_keyid;
        u64 access_key_secret;
        u64 project;
        u64 log_store;
    } hash_struct{};

    hash_struct.topic = CityHash64(topic.c_str(), topic.length());
    hash_struct.endpoint = CityHash64(endpoint.c_str(), endpoint.length());
    hash_struct.access_keyid = CityHash64(access_keyid.c_str(), access_keyid.length());
    hash_struct.access_key_secret = CityHash64(access_key_secret.c_str(), access_key_secret.length());
    hash_struct.project = CityHash64(project.c_str(), project.length());
    hash_struct.log_store = CityHash64(log_store.c_str(), log_store.length());

    hash = CityHash64(reinterpret_cast<const char*>(&hash_struct), sizeof(hash_struct));
    return hash;
}

std::shared_ptr<TrackMessage> CreateTracker(const std::shared_ptr<TrackerConfig> &config) {
    return tracker::CreateTracker(config);
}

bool FlushTracker(const std::shared_ptr<TrackMessage>& tracker) { return tracker::FlushTracker(tracker); }

}  // namespace tapsdk
