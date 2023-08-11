//
// Created by 甘尧 on 2023/6/29.
//

#pragma once

#include "sdk/tapsdk.h"

namespace tapsdk::bindings {

#define DEV_TYPE_LOCAL 0
#define DEV_TYPE_SANDBOX 1
#define DEV_TYPE_CLOUD 2

struct BridgeConfig {
    bool enable_duration_statistics{true};
    std::string device_id{};
    std::string cache_dir{};
    std::string ca_dir{};
    int device_type{DEV_TYPE_LOCAL};
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