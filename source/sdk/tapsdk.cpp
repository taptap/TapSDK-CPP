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

static std::shared_ptr<Game> current_game;

// modules
static std::unique_ptr<duration::DurationStatistics> duration_statistics;

bool Init(const Config& config) {
    sdk_config = config;
    Runtime::Get().Init();
    if (sdk_config.enable_duration_statistics) {
        duration_statistics = std::make_unique<duration::DurationStatistics>();
        duration_statistics->Init();
    }
    return true;
}

void TDSUser::SetCurrent(const std::shared_ptr<TDSUser>& user) {
    std::scoped_lock guard(user_lock);
    current_user = user;
    Runtime::Get().GetEventBus()->notifyNow(events::User{user});
}

std::shared_ptr<TDSUser> TDSUser::GetCurrent() { return current_user; }

std::string TDSUser::GetUserId() { return user_id; }

TDSUser::TDSUser(const std::string& id)
        : user_id{id} {}

void Game::SetCurrent(const std::shared_ptr<Game>& game) {
    current_game = game;
    Runtime::Get().GetEventBus()->notifyNow(events::GameStart{game});
}

std::shared_ptr<Game> Game::GetCurrent() {
    return current_game;
}

Future<TDSUser> Login(const char *name, const char *passwd) {
    Future<TDSUser> future{};
    return future;
}

}  // namespace tapsdk
