//
// Created by 甘尧 on 2023/7/10.
//

#include "model.h"

namespace tapsdk::duration {

const char *ActionString(EventAction action) {
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

ReportConfig::ReportConfig(const net::Json &json) {
    enable = json["enable"];
    no_tap_enable = json["notap_enable"];
    tap_frequency = json["tap_frequency"];
    no_tap_frequency = json["notap_frequency"];
    available_start_ts = json["available_start_ts"];
    server_ts = json["server_ts"];
}

u64 ReportConfig::ServerTimestamp() const { return server_ts; }

net::Json ReportContent::ToJson() {
    net::Json json{};
    json["event"] = ActionString(static_cast<EventAction>(event.action));
    json["ts"] = event.timestamp / 1000;
    json["client_id"] = event.game_id;
    json["identifier"] = event.game_pkg;
    net::Json user{};
    if (event.tap_user) {
        user["tds_id"] = event.user_id;
    } else {
        user["open_id"] = event.user_id;
    }
    json["user"] = user;
    json["device_id"] = event.device_id;
    net::Json extra{};
    extra["session_id"] = event.session;
    if (event.last_timestamp) {
        extra["last_ts"] = event.last_timestamp / 1000;
    }
    json["extra"] = extra;
    return std::move(json);
}

}
