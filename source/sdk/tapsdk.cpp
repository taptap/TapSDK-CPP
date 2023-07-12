//
// Created by 甘尧 on 2023/6/29.
//

#include "base/logging.h"
#include "core/events.h"
#include "core/runtime.h"
#include "duration/duration.h"
#include "tapsdk.h"

namespace tapsdk {

static Config sdk_config;

// users
static std::mutex user_lock;
static std::shared_ptr<TDSUser> current_user;

// modules
static std::unique_ptr<duration::DurationStatistics> duration_statistics;

bool Init(const Config &config) {
    sdk_config = config;
    Runtime::Get().Init();
    if (sdk_config.enable_duration_statistics) {
        duration_statistics = std::make_unique<duration::DurationStatistics>();
        duration_statistics->Init();
    }
    return true;
}

void TDSUser::SetCurrent(TDSUserHandle user) {
    std::scoped_lock guard(user_lock);
    current_user = user;
    Runtime::Get().GetEventBus()->notifyNow(events::User{user});
}

TDSUserHandle TDSUser::GetCurrent() {
    return current_user;
}

std::string TDSUser::GetUserId() {
    return user_id;
}

std::string TDSUser::GetUserName() {
    return user_name;
}
}
