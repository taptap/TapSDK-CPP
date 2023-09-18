//
// Created by 甘尧 on 2023/9/14.
//

#pragma once

#include "net/network.h"

namespace tapsdk::tracker {

namespace ReportKey {
constexpr auto SDK_VERSION = "sdk_version";
constexpr auto SDK_VERSION_NAME = "sdk_version_name";
constexpr auto DEVICE_ID = "device_id";
constexpr auto T_LOG_ID = "t_log_id";
constexpr auto VERSION = "version";
// 厂商名称
constexpr auto DEVICE_VERSION = "dv";
constexpr auto MODEL = "md";
constexpr auto CPU = "cpu";
constexpr auto APP_PACKAGE_NAME = "app_package_name";
constexpr auto APP_VERSION = "app_version";
constexpr auto NETWORK_TYPE = "network_type";
constexpr auto MOBILE_TYPE = "mobile_type";
constexpr auto OS_PARAM = "os";
constexpr auto SYSTEM_VERSION = "sv";
constexpr auto RAM = "ram";
constexpr auto ROM = "rom";
// 时间
constexpr auto TIME = "time";
}  // namespace ReportKey

struct UploadResult {
    explicit UploadResult(const net::Json &json) {}
};

}  // namespace tapsdk::tracker