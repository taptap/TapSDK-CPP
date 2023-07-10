//
// Created by 甘尧 on 2023/7/7.
//

#include "base/logging.h"
#include "duration.h"
#include "sdk/platform.h"

namespace tapsdk::duration {

void DurationStatistics::Init() {
    auto cur_device = platform::Device::GetCurrent();
    ASSERT_MSG(cur_device, "Please set current device first!");
    persistence = std::make_unique<DurPersistence>(cur_device->GetCacheDir());
    tick_event = Event::Create([this](Duration later, std::uintptr_t user_data) {
        Runtime::Get().Timer().PostEvent(tick_event, tick_interval);
    });
    Runtime::Get().Timer().PostEvent(tick_event, tick_interval);
}

}  // namespace tapsdk::duration
