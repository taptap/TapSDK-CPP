//
// Created by 甘尧 on 2023/7/10.
//

#pragma once

#include "base/types.h"
#include "net/httpclient.h"
#include "sdk/platform.h"

namespace tapsdk::duration {
class DurPersistence;

enum EventAction {
    NULL_EVENT = 0,
    GAME_START,
    GAME_FOREGROUND,
    GAME_BACKGROUND,
    GAME_END,
    HEAT_BEAT,
    USER_LOGIN,
    USER_LOGOUT,
};

struct DurEvent {
    u32 id = 0;
    u32 action;
    bool tap_user;
    std::string user_id;
    std::string game_id;
    std::string game_pkg;
    std::string device_id;
    std::string session;
    u64 timestamp;
    u64 last_timestamp{};
    platform::DeviceType dev_type{platform::DeviceType::Local};
    std::shared_ptr<platform::DeviceInfo> device_info{};
    std::string sdk_version;
};

struct GameSession {
    u32 id = 0;
    bool tap_user;
    std::string session;
    std::string user_id;
    std::string game_id;
    std::string game_pkg;
    u64 game_start;
    u64 last_beats;
    u64 last_timestamp{};
};

class ReportResult {
public:
    explicit ReportResult(const net::Json& json){};
};

class ReportConfig {
public:
    explicit ReportConfig() = default;

    explicit ReportConfig(const net::Json& json);

    [[nodiscard]] u64 ServerTimestamp() const;

    [[nodiscard]] bool Enabled() const;

    [[nodiscard]] bool NoTapEnabled() const;

    [[nodiscard]] u32 TapFrequency() const;

    [[nodiscard]] u32 NoTapFrequency() const;

    u32 id = 0;
    bool enable = true;
    bool no_tap_enable = true;
    u32 tap_frequency = 150;
    u32 no_tap_frequency = 300;
    u64 available_start_ts = 0;
    u64 server_ts = 0;
};

class ReportContent {
public:
    explicit ReportContent(DurEvent& event) : event{event} {}

    net::Json ToJson();

private:
    DurEvent& event;
};

}  // namespace tapsdk::duration