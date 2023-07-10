//
// Created by 甘尧 on 2023/7/7.
//
#pragma once

#include <memory>
#include <string>
#include <cstdint>

namespace tapsdk::platform {

class Window {
public:
    static void OnForeground();
    static void OnBackground();
};

class Device {
public:
    static void SetCurrent(const std::shared_ptr<Device> &device);

    virtual std::string GetDeviceID() = 0;
    virtual std::string GetCacheDir() = 0;
};

}
