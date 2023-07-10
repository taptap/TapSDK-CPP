//
// Created by 甘尧 on 2023/7/7.
//

#pragma once

#include "base/timer.h"
#include "core/runtime.h"
#include "duration/persistence.h"

namespace tapsdk::duration {

constexpr auto default_tick_interval = Ms(10 * 1000);

class DurationStatistics {
public:
    void Init();

private:
    std::shared_ptr<Event> tick_event;
    Duration tick_interval{default_tick_interval};
    std::unique_ptr<DurPersistence> persistence{};
    dexode::EventBus::Listener event_listener{Runtime::Get().GetEventBus()};
};

}  // namespace tapsdk::duration
