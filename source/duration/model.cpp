//
// Created by 甘尧 on 2023/7/10.
//

#include "model.h"

namespace tapsdk::duration {

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
    json["action"] = event.action;
    json["timestamp"] = event.timestamp;
    json["game_id"] = event.game_id;
    json["user_id"] = event.user_id;
    return std::move(json);
}

}
