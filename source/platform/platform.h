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

}
