//
// Created by 甘尧 on 2023/7/7.
//

#pragma once

#include "base/timer.h"
#include "dexode/EventBus.hpp"

namespace tapsdk {

class Runtime {
public:
    static Runtime& Get();

    void Init();

    CoreTimer& Timer() { return timer; }

    std::shared_ptr<dexode::EventBus> &GetEventBus() { return event_bus; }

private:
    CoreTimer timer;
    std::shared_ptr<dexode::EventBus> event_bus;
};

}  // namespace tapsdk
