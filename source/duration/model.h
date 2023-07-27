//
// Created by 甘尧 on 2023/7/10.
//

#pragma once

#include "base/types.h"
#include "net/httpclient.h"

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
};

class ReportResult {
public:
    explicit ReportResult(const net::Json& json){};
};

class ReportConfig {
public:
    explicit ReportConfig(const net::Json& json);

    [[nodiscard]] u64 ServerTimestamp() const;

private:
    bool enable;
    bool no_tap_enable;
    u32 tap_frequency;
    u32 no_tap_frequency;
    u64 available_start_ts;
    u64 server_ts;
};

class ReportContent {
public:
    explicit ReportContent(DurEvent& event) : event{event} {}

    net::Json ToJson();

private:
    DurEvent& event;
};

}  // namespace tapsdk::duration