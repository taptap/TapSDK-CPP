//
// Created by 甘尧 on 2023/11/14.
//

#include <unordered_map>
#include <mutex>
#include "base/types.h"
#include "platform.h"
#include "tapsdk.h"
#include "tapsdk_api.h"

#define R_UNLESS(expr, res)                                                                        \
    {                                                                                              \
        if (!(expr)) {                                                                             \
            return res;                                                                            \
        }                                                                                          \
    }

static std::string ToString(const char* str) { return str ? str : std::string{}; }

namespace tapsdk::api {

struct TrackMessage : tapsdk_tracker_message {
    std::shared_ptr<tapsdk::TrackMessage> message;
};

std::shared_ptr<TrackerConfig> CreateConfig(tapsdk_tracker_config& config) {
    auto result = std::make_shared<TrackerConfig>();
    result->topic = ToString(config.topic);
    result->endpoint = ToString(config.endpoint);
    result->access_keyid = ToString(config.access_keyid);
    result->access_key_secret = ToString(config.access_key_secret);
    result->project = ToString(config.project);
    result->log_store = ToString(config.log_store);
    result->sdk_version_name = ToString(config.sdk_version_name);
    config.hash = result->Hash();
    return result;
}

class Device : public platform::Device {
public:
    explicit Device(tapsdk_device* config) {
        type = config->type;
        device_id = ToString(config->device_id);
        cache_dir = ToString(config->cache_dir);
        device_info = std::make_shared<platform::DeviceInfo>();
        if (config->info) {
            device_info->device_version = ToString(config->info->device_version);
            device_info->model = ToString(config->info->model);
            device_info->platform = ToString(config->info->platform);
            device_info->engine = ToString(config->info->engine);
            device_info->os_version = ToString(config->info->os_version);
            device_info->android_id = ToString(config->info->android_id);
            device_info->ram_size = ToString(config->info->ram_size);
            device_info->rom_size = ToString(config->info->rom_size);
            device_info->network_type = ToString(config->info->network_type);
            device_info->mobile_type = ToString(config->info->mobile_type);
            device_info->cpu_info = ToString(config->info->cpu_info);
        }
    }

    std::string GetDeviceID() override { return device_id; }

    std::string GetCacheDir() override { return cache_dir; }

    std::string GetCaCertDir() override { return ""; }

    platform::DeviceType GetDeviceType() override {
        switch (type) {
            case TDS_DEV_TYPE_LOCAL:
                return platform::DeviceType::Local;
            case TDS_DEV_TYPE_SANDBOX:
                return platform::DeviceType::Sandbox;
            case TDS_DEV_TYPE_CLOUD:
                return platform::DeviceType::Cloud;
            default:
                throw std::runtime_error("UnSupport Device!");
        }
    }

    std::shared_ptr<platform::DeviceInfo> GetDeviceInfo() override { return device_info; }

private:
    int type;
    std::string device_id;
    std::string cache_dir;
    std::shared_ptr<platform::DeviceInfo> device_info;
};

class Game : public tapsdk::Game {
public:
    explicit Game(tapsdk_game* info) {
        client_id = ToString(info->client_id);
        identify = ToString(info->identify);
        app_version = ToString(info->app_version);
    }

    std::string GetGameID() override { return client_id; }

    std::string GetPackageName() override { return identify; }

    std::string GetVersion() override { return app_version; }

private:
    std::string client_id;
    std::string identify;
    std::string app_version;
};

class User : public TDSUser {
public:
    explicit User(tapsdk_user* info) : info(info) {}

    std::string GetUserId() override { return ToString(info->user_id); }

    std::string GetUserName() override { return ""; }

    bool ContainTapInfo() override { return info->contain_tap_info; }

private:
    tapsdk_user* info;
};

}  // namespace tapsdk::api

extern "C" {

using namespace tapsdk;

static bool sdk_inited{false};

tapsdk_result tapsdk_init(tapsdk_config* config) {
    R_UNLESS(config, TAPSDK_INVALID_INPUT)
    R_UNLESS(config->device, TAPSDK_INVALID_INPUT)
    R_UNLESS(config->region >= TDS_REGION_CN && config->region <= TDS_REGION_RND,
             TAPSDK_INVALID_INPUT)
    R_UNLESS(config->device->type >= TDS_DEV_TYPE_LOCAL &&
                     config->device->type <= TDS_DEV_TYPE_CLOUD,
             TAPSDK_INVALID_INPUT)
    platform::Device::SetCurrent(std::make_shared<api::Device>(config->device));
    Config cof{
            .enable_duration_statistics = config->enable_duration_statistics,
            .enable_tap_login = config->enable_tap_login,
            .enable_tap_tracker = config->enable_tap_tracker,
            .client_id = ToString(config->client_id),
            .process_name = ToString(config->process_name),
            .region = static_cast<Region>(config->region),
            .sdk_version = ToString(config->sdk_version),
            .tracker_group_size = config->tracker_group_size,
    };
    if (Init(cof)) {
        sdk_inited = true;
        return TAPSDK_SUCCESS;
    } else {
        return TAPSDK_ERROR;
    }
}

tapsdk_result tapsdk_game_set(tapsdk_game* game) {
    R_UNLESS(sdk_inited, TAPSDK_NO_INIT)
    if (game) {
        R_UNLESS(game->client_id, TAPSDK_INVALID_INPUT)
        R_UNLESS(game->identify, TAPSDK_INVALID_INPUT)
        Game::SetCurrent(std::make_shared<api::Game>(game));
    } else {
        Game::SetCurrent({});
    }
    return TAPSDK_SUCCESS;
}

tapsdk_result tapsdk_user_set(tapsdk_user* user) {
    R_UNLESS(sdk_inited, TAPSDK_NO_INIT)
    if (user) {
        R_UNLESS(user->user_id, TAPSDK_NO_INIT)
        TDSUser::SetCurrent(std::make_shared<api::User>(user));
    } else {
        TDSUser::SetCurrent({});
    }
    return TAPSDK_SUCCESS;
}

tapsdk_result tapsdk_window_on_foreground() {
    R_UNLESS(sdk_inited, TAPSDK_NO_INIT)
    platform::Window::OnForeground();
    return TAPSDK_SUCCESS;
}

tapsdk_result tapsdk_window_on_background() {
    R_UNLESS(sdk_inited, TAPSDK_NO_INIT)
    platform::Window::OnBackground();
    return TAPSDK_SUCCESS;
}

static std::mutex tracker_lock{};
static std::unordered_map<u64, std::shared_ptr<TrackerConfig>> tracker_configs{};

tapsdk_result tapsdk_tracker_create(tapsdk_tracker_config* config,
                                    tapsdk_tracker_message** message) {
    R_UNLESS(sdk_inited, TAPSDK_NO_INIT)
    R_UNLESS(config, TAPSDK_INVALID_INPUT)
    R_UNLESS(message, TAPSDK_INVALID_INPUT)
    auto& hash = config->hash;

    auto msg = new api::TrackMessage();
    std::scoped_lock guard(tracker_lock);
    std::shared_ptr<TrackerConfig> tracker_config;
    if (!hash) {
        tracker_config = api::CreateConfig(*config);
        tracker_configs[tracker_config->hash] = tracker_config;
    } else {
        tracker_config = tracker_configs[hash];
        if (!tracker_config) {
            tracker_config = api::CreateConfig(*config);
        }
    }
    msg->message = CreateTracker(tracker_config);
    if (msg->message) {
        *message = msg;
        return TAPSDK_SUCCESS;
    } else {
        return TAPSDK_ERROR;
    }
}

tapsdk_result tapsdk_tracker_msg_add_param(tapsdk_tracker_message* message,
                                                   const char* key,
                                                   const char* value) {
    R_UNLESS(sdk_inited, TAPSDK_NO_INIT)
    R_UNLESS(message, TAPSDK_INVALID_INPUT)
    R_UNLESS(key, TAPSDK_INVALID_INPUT)
    R_UNLESS(value, TAPSDK_INVALID_INPUT)
    auto api_msg = reinterpret_cast<api::TrackMessage*>(message);
    R_UNLESS(api_msg->message, TAPSDK_INVALID_INPUT)
    api_msg->message->AddParam(key, value);
    return TAPSDK_SUCCESS;
}

tapsdk_result tapsdk_tracker_msg_add_content(tapsdk_tracker_message* message,
                                                     const char* key,
                                                     const char* value) {
    R_UNLESS(sdk_inited, TAPSDK_NO_INIT)
    R_UNLESS(message, TAPSDK_INVALID_INPUT)
    R_UNLESS(key, TAPSDK_INVALID_INPUT)
    R_UNLESS(value, TAPSDK_INVALID_INPUT)
    auto api_msg = reinterpret_cast<api::TrackMessage*>(message);
    R_UNLESS(api_msg->message, TAPSDK_INVALID_INPUT)
    api_msg->message->AddContent(key, value);
    return TAPSDK_SUCCESS;
}

tapsdk_result tapsdk_tracker_flush(tapsdk_tracker_message* message) {
    R_UNLESS(sdk_inited, TAPSDK_NO_INIT)
    R_UNLESS(message, TAPSDK_INVALID_INPUT)
    auto api_msg = reinterpret_cast<api::TrackMessage*>(message);
    R_UNLESS(api_msg->message, TAPSDK_INVALID_INPUT)
    auto result = FlushTracker(api_msg->message);
    if (result) {
        delete api_msg;
        return TAPSDK_SUCCESS;
    } else {
        return TAPSDK_ERROR;
    }
}
}