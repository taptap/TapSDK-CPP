//
// Created by 甘尧 on 2023/7/7.
//

#include "base/logging.h"
#include "core/events.h"
#include "duration.h"
#include "sdk/platform.h"
#include "sdk/tapsdk.h"
#include "net/network.h"

namespace tapsdk::duration {

void DurationStatistics::Init() {
    auto cur_device = platform::Device::GetCurrent();
    ASSERT_MSG(cur_device, "Please set current device first!");
    persistence = std::make_unique<DurPersistence>(cur_device->GetCacheDir());
    http_client = net::CreateHttpClient("", true);
    event_listener.listen([] (const events::User &new_user) {
        // TODO
        LOG_ERROR("OnNewUser");
    });
    event_listener.listen([] (const events::Foreground &new_user) {
        // TODO
        LOG_ERROR("OnForeground");
    });
    event_listener.listen([] (const events::Background &new_user) {
        // TODO
        LOG_ERROR("OnBackground");
    });
    tick_event = Event::Create([this](Duration later, std::uintptr_t user_data) {
        Runtime::Get().Timer().PostEvent(tick_event, tick_interval);
    });
    Runtime::Get().Timer().PostEvent(tick_event, tick_interval);
    InitRequest();
}

void DurationStatistics::InitRequest() {

}

}  // namespace tapsdk::duration
