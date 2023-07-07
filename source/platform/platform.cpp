//
// Created by 甘尧 on 2023/7/7.
//

#include "base/logging.h"
#include "sdk/platform.h"

namespace tapsdk::platform {

void Window::OnForeground() {
    LOG_ERROR("OnForeground");
}

void Window::OnBackground() {
    LOG_ERROR("OnBackground");
}

}
