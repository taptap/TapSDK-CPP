//
// Created by 甘尧 on 2023/7/10.
//

#include "dur_model.h"

namespace tapsdk::duration {

const char* ActionString(EventAction action) {
    switch (action) {
        case GAME_START:
            return "start";
        case GAME_END:
            return "end";
        case HEAT_BEAT:
            return "heartbeat";
        case USER_LOGIN:
            return "login";
        case USER_LOGOUT:
            return "logout";
        case GAME_FOREGROUND:
            return "resume";
        case GAME_BACKGROUND:
            return "pause";
        default:
            return "";
    }
}

const char* DevTypeString(platform::DeviceType type) {
    switch (type) {
        case platform::DeviceType::Local:
            return "local";
        case platform::DeviceType::Sandbox:
            return "sandbox";
        case platform::DeviceType::Cloud:
            return "cloud";
        default:
            return "";
    }
}

ReportConfig::ReportConfig(const net::Json& json) {
    enable = json["enable"];
    no_tap_enable = json["notap_enable"];
    tap_frequency = json["tap_frequency"];
    no_tap_frequency = json["notap_frequency"];
    available_start_ts = json["available_start_ts"];
    server_ts = json["server_ts"];
}

u64 ReportConfig::ServerTimestamp() const { return server_ts; }

bool ReportConfig::Enabled() const { return enable; }

bool ReportConfig::NoTapEnabled() const { return no_tap_enable; }

u32 ReportConfig::NoTapFrequency() const { return no_tap_frequency; }

u32 ReportConfig::TapFrequency() const { return tap_frequency; }

net::Json ReportContent::ToJson() {
    net::Json json{};
    json["event"] = ActionString(static_cast<EventAction>(event.action));
    json["ts"] = event.timestamp / 1000;
    json["client_id"] = event.game_id;
    json["identifier"] = event.game_pkg;
    if (!event.user_id.empty()) {
        try {
            net::Json user = net::Json::parse(event.user_id);
            json["user"] = user;
        } catch (std::exception &e) {
            throw std::runtime_error(fmt::format("Error user json: {}!", e.what()));
        }
    }
    json["device_id"] = event.device_id;
    net::Json extra{};
    extra["session_id"] = event.session;
    if (event.last_timestamp) {
        extra["last_ts"] = event.last_timestamp / 1000;
    }
    json["extra"] = extra;
    json["environment"] = DevTypeString(event.dev_type);
    json["sdk_version"] = event.sdk_version;
    if (event.device_info) {
        json["os"] = event.device_info->platform;
        json["engine"] = event.device_info->engine;
        json["device_model"] = event.device_info->model;
    }
    return std::move(json);
}

}  // namespace tapsdk::duration
