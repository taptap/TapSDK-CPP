//
// Created by 甘尧 on 2023/7/7.
//

#include "platform.h"
#include "base/logging.h"

namespace tapsdk::platform {

void Window::OnForeground() {
    LOG_ERROR("OnForeground");
}

void Window::OnBackground() {
    LOG_ERROR("OnBackground");
}

}
