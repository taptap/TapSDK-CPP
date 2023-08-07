//
// Created by 甘尧 on 2023/6/29.
//

#pragma once

#include "sdk/tapsdk.h"

namespace tapsdk::bindings {

struct BridgeConfig {
    bool enable_duration_statistics{true};
    std::string device_id{};
    std::string cache_dir{};
    std::string ca_dir{};
};

struct BridgeUser {
    bool contain_tap_info{false};
    std::string user_id{};
};

struct BridgeGame {
    std::string client_id{};
    std::string identify{};
};

void InitSDK(BridgeConfig &config);
void SetCurrentGame(BridgeGame &game);
void SetCurrentUser(BridgeUser &user);

void OnWindowForeground();
void OnWindowBackground();

}