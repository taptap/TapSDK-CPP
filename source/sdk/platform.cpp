//
// Created by 甘尧 on 2023/7/7.
//

#include "base/logging.h"
#include "core/events.h"
#include "core/runtime.h"
#include "sdk/platform.h"

namespace tapsdk::platform {

static std::shared_ptr<Device> current_device{};
static std::shared_ptr<Window> current_window{};

void Window::OnForeground() {
    Runtime::Get().GetEventBus()->notifyNow(events::Foreground{});
}

void Window::OnBackground() {
    Runtime::Get().GetEventBus()->notifyNow(events::Background{});
}

void Device::SetCurrent(const std::shared_ptr<Device> &device) {
    current_device = device;
}

std::shared_ptr<Device> Device::GetCurrent() {
    return current_device;
}

DeviceType Device::GetDeviceType() {
    return DeviceType::Local;
}

void Window::SetCurrent(const std::shared_ptr<Window>& window) {
    current_window = window;
}

std::shared_ptr<Window> Window::GetCurrent() {
    return current_window;
}

}
