//
// Created by 甘尧 on 2023/6/29.
//

#include <unistd.h>
#include "base/logging.h"
#include "core/events.h"
#include "core/runtime.h"
#include "duration/duration.h"
#include "tapsdk.h"

namespace tapsdk {

// users
std::mutex user_lock;
std::shared_ptr<TDSUser> current_user;

// modules
std::unique_ptr<duration::DurationStatistics> duration_statistics;

bool Init() {
    Runtime::Get().Init();
    duration_statistics = std::make_unique<duration::DurationStatistics>();
    duration_statistics->Init();
    return true;
}

void TDSUser::SetCurrent(TDSUserHandle user) {
    LOG_ERROR("UserName {}", user->GetUserName());
    std::scoped_lock guard(user_lock);
    current_user = user;
    Runtime::Get().GetEventBus()->notifyNow(UserEvent{user});
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

void Login(const char* account, const char* passwd, LoginCallback *cb) {
    cb->OnFailed(-1, "Unimplemented");
}

}
