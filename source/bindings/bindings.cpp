//
// Created by 甘尧 on 2023/6/29.
//

#include "bindings.h"

#include <utility>
#include "sdk/platform.h"
#include "sdk/tapsdk.h"

namespace tapsdk::bindings {

class BDevice : public platform::Device {
public:
    explicit BDevice(BridgeConfig config) : config(std::move(config)) {}

    std::string GetDeviceID() override { return config.device_id; }

    std::string GetCacheDir() override { return config.cache_dir; }

    std::string GetCaCertDir() override { return config.ca_dir; }

    platform::DeviceType GetDeviceType() override {
        switch (config.device_type) {
            case DEV_TYPE_LOCAL:
                return platform::DeviceType::Local;
            case DEV_TYPE_SANDBOX:
                return platform::DeviceType::Sandbox;
            case DEV_TYPE_CLOUD:
                return platform::DeviceType::Cloud;
            default:
                throw std::runtime_error("UnSupport Device!");
        }
    }

    std::shared_ptr<platform::DeviceInfo> GetDeviceInfo() override {
        if (config.device_info) {
            return config.device_info;
        } else {
            return std::make_shared<platform::DeviceInfo>();
        }
    }

private:
    BridgeConfig config;
};

class BGame : public tapsdk::Game {
public:
    explicit BGame(BridgeGame info) : game_info(std::move(info)) {}

    std::string GetGameID() override { return game_info.client_id; }

    std::string GetPackageName() override { return game_info.identify; }

private:
    BridgeGame game_info;
};

class BUser : public TDSUser {
public:
    explicit BUser(BridgeUser info) : info(std::move(info)) {}

    std::string GetUserId() override { return info.user_id; }

    std::string GetUserName() override { return ""; }

    bool ContainTapInfo() override { return info.contain_tap_info; }

private:
    BridgeUser info;
};

void InitSDK(BridgeConfig& config) {
    platform::Device::SetCurrent(std::make_shared<BDevice>(config));
    Config conf{.enable_duration_statistics = config.enable_duration_statistics,
                .region = config.region,
                .sdk_version = config.sdk_version};
    Init(conf);
}

void SetCurrentUser(BridgeUser* user) {
    if (user) {
        TDSUser::SetCurrent(std::make_shared<BUser>(*user));
    } else {
        TDSUser::SetCurrent({});
    }
}

void SetCurrentGame(BridgeGame& game) { Game::SetCurrent(std::make_shared<BGame>(game)); }

void OnWindowForeground() { platform::Window::OnForeground(); }

void OnWindowBackground() { platform::Window::OnBackground(); }

}  // namespace tapsdk::bindings
