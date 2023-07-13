//
// Created by 甘尧 on 2023/7/7.
//

#include "base/logging.h"
#include "base/uuid.h"
#include "core/events.h"
#include "duration.h"
#include "sdk/platform.h"

namespace tapsdk::duration {

void DurationStatistics::Init() {
    auto cur_device = platform::Device::GetCurrent();
    ASSERT_MSG(cur_device, "Please set current device first!");
    persistence = std::make_unique<DurPersistence>(cur_device->GetCacheDir());
    http_client = net::CreateHttpClient("", true);
    NewGameSession();
    InitEvents();
    InitHeatBeats();
    InitRequest();
    InitReportThread();
}

void DurationStatistics::InitEvents() {
    event_listener.listen([this](const events::User& new_user) { OnNewUser(new_user.user); });
    event_listener.listen([this](const events::Foreground& foreground) { OnForeground(); });
    event_listener.listen([this](const events::Background& background) { OnBackground(); });
    event_listener.listen(
            [this](const events::GameStart& game_start) { GameStart(game_start.game->GetUUID()); });
}

void DurationStatistics::InitHeatBeats() {
    auto &timer = Runtime::Get().Timer();
    local_heat_beat = Event::Create([&timer, this](Duration later, std::uintptr_t user_data) {
        if (!game_uuid.empty()) {
            DurEvent event{.action = HEAT_BEAT,
                           .user_id = user_id,
                           .game_id = game_uuid,
                           .session = game_session,
                           .timestamp = Timestamp()};
            persistence->AddOrMergeEvent(event);
        }
        timer.PostEvent(local_heat_beat, local_heat_beat_interval);
    });
    online_heat_beat = Event::Create([&timer, this](Duration later, std::uintptr_t user_data) {
        timer.PostEvent(online_heat_beat, online_tick_interval);
    });
    timer.PostEvent(local_heat_beat, local_heat_beat_interval);
    timer.PostEvent(online_heat_beat, online_tick_interval);
}

void DurationStatistics::InitReportThread() {
    report_thread = std::make_unique<std::thread>([this]() {
        while (running) {
            auto event = report_queue.Take();
            auto now = Runtime::Get().Timer().TimeEpoch();
            if (now - latest_online_heat_beats < online_tick_interval) {
                continue;
            }
            if (event.action == HEAT_BEAT) {
                latest_online_heat_beats = now;
            }

            // do report
        }
    });
}

void DurationStatistics::InitRequest() {}

void DurationStatistics::NewGameSession() {
    // pre game session
    auto latest_event = persistence->GetLatestEvent();
    if (latest_event && latest_event->action != GAME_END) {
        // new game end event
        auto new_event = *latest_event;
        new_event.action = GAME_END;
        persistence->AddOrMergeEvent(new_event);
        report_queue.Put(new_event);
    }

    // new game
    game_session = CreateUUID();
}

void DurationStatistics::GameStart(const std::string& uuid) {
    game_uuid = uuid;
    DurEvent event{.action = GAME_START,
                   .user_id = user_id,
                   .game_id = game_uuid,
                   .session = game_session,
                   .timestamp = Timestamp()};
    persistence->AddOrMergeEvent(event);
    report_queue.Put(event);
}

void DurationStatistics::OnBackground() {
    DurEvent event{.action = GAME_BACKGROUND,
                   .user_id = user_id,
                   .game_id = game_uuid,
                   .session = game_session,
                   .timestamp = Timestamp()};
    persistence->AddOrMergeEvent(event);
    report_queue.Put(event);
}

void DurationStatistics::OnForeground() {
    DurEvent event{.action = GAME_FOREGROUND,
                   .user_id = user_id,
                   .game_id = game_uuid,
                   .session = game_session,
                   .timestamp = Timestamp()};
    persistence->AddOrMergeEvent(event);
    report_queue.Put(event);
}

void DurationStatistics::OnNewUser(const std::shared_ptr<TDSUser>& user) {
    if (user) {
        // login
        user_id = user->GetUserId();
        DurEvent event{.action = USER_LOGIN,
                       .user_id = user_id,
                       .game_id = game_uuid,
                       .session = game_session,
                       .timestamp = Timestamp()};
        persistence->AddOrMergeEvent(event);
        report_queue.Put(event);
    } else {
        // logout
        user_id = "";
        DurEvent event{.action = USER_LOGOUT,
                       .user_id = user_id,
                       .game_id = game_uuid,
                       .session = game_session,
                       .timestamp = Timestamp()};
        persistence->AddOrMergeEvent(event);
        report_queue.Put(event);
    }
}

static_assert(sizeof(std::time_t) == sizeof(u64));
u64 DurationStatistics::Timestamp() {
    std::time_t time_stamp = std::time(nullptr);
    return time_stamp;
}

DurationStatistics::~DurationStatistics() {
    if (report_thread) {
        running = false;
        report_queue.Put({});
        report_thread->join();
    }
}

}  // namespace tapsdk::duration
