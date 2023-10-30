//
// Created by 甘尧 on 2023/7/7.
//
#pragma once

#include <memory>
#include <string>
#include <cstdint>

namespace tapsdk::platform {

enum class DeviceType {
    Local,
    Sandbox,
    Cloud
};

class Window {
public:
    // 当 App 进入前台
    static void OnForeground();
    // 当 App 进入后台
    static void OnBackground();
};

struct DeviceInfo {
    std::string device_version = "";
    std::string model = "";
    std::string platform = "";
    std::string engine = "";
    std::string os_version = "";
    std::string android_id = "";
    std::string app_version = "";
    std::string app_package_name = "";
    std::string ram_size = "";
    std::string rom_size = "";
    std::string network_type = "";
    std::string mobile_type = "";
    std::string cpu_info = "";
};

class Device {
public:
    static void SetCurrent(const std::shared_ptr<Device> &device);
    static std::shared_ptr<Device> GetCurrent();

    virtual ~Device() = default;

    // 当前 Device ID
    virtual std::string GetDeviceID() = 0;
    // 缓存目录
    virtual std::string GetCacheDir() = 0;
    // CA 证书目录 (可选)
    virtual std::string GetCaCertDir() = 0;
    // 设备类型
    virtual DeviceType GetDeviceType();
    // 设备详细信息
    virtual std::shared_ptr<DeviceInfo> GetDeviceInfo() = 0;
};

}
