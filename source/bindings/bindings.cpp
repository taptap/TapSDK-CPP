//
// Created by 甘尧 on 2023/6/29.
//

#include "bindings.h"
#include "sdk/tapsdk.h"
#include "sdk/platform.h"

namespace tapsdk::bindings {

class BDevice : public platform::Device {
public:

    BDevice(const BridgeConfig &config) : config(config) {}

    std::string GetDeviceID() override {
        return config.device_id;
    }

    std::string GetCacheDir() override {
        return config.cache_dir;
    }

    std::string GetCaCertDir() override {
        return config.ca_dir;
    }

private:
    BridgeConfig config;
};

class BGame : public tapsdk::Game {
public:

    BGame(const BridgeGame &info) : game_info(info) {}

    std::string GetGameID() override {
        return game_info.client_id;
    }

    std::string GetPackageName() override {
        return game_info.identify;
    }

private:
    BridgeGame game_info;
};

class BUser : public TDSUser {
public:

    BUser(const BridgeUser &info) : info(info) {}

    std::string GetUserId() override {
        return info.user_id;
    }

    std::string GetUserName() override {
        return "";
    }

    bool ContainTapInfo() override {
        return info.contain_tap_info;
    }

private:
    BridgeUser info;
};

void InitSDK(BridgeConfig &config) {
    platform::Device::SetCurrent(std::make_shared<BDevice>(config));
    Config conf {
        config.enable_duration_statistics = config.enable_duration_statistics
    };

    Init(conf);
}

void SetCurrentUser(BridgeUser &user) {
    TDSUser::SetCurrent(std::make_shared<BUser>(user));
}

void SetCurrentGame(BridgeGame &game) {
    Game::SetCurrent(std::make_shared<BGame>(game));
}

void OnWindowForeground() {
    platform::Window::OnForeground();
}

void OnWindowBackground() {
    platform::Window::OnBackground();
}

}
