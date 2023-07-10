//
// Created by 甘尧 on 2023/7/7.
//

#include "base/logging.h"
#include "sdk/platform.h"

namespace tapsdk::platform {

static std::shared_ptr<Device> current_device;

void Window::OnForeground() {
    LOG_ERROR("OnForeground");
}

void Window::OnBackground() {
    LOG_ERROR("OnBackground");
}

void Device::SetCurrent(const std::shared_ptr<Device> &device) {
    LOG_ERROR("DeviceID {}", device->GetDeviceID());
    current_device = device;
}

}
