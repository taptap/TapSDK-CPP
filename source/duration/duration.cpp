//
// Created by 甘尧 on 2023/7/7.
//

#include "base/logging.h"
#include "base/thread.h"
#include "base/uuid.h"
#include "core/events.h"
#include "duration.h"
#include "sdk/platform.h"

namespace tapsdk::duration {

void DurationStatistics::Init() {
    auto cur_device = platform::Device::GetCurrent();
    ASSERT_MSG(cur_device, "Please set current device first!");
    persistence = std::make_unique<DurPersistence>(cur_device->GetCacheDir());
    http_client = net::CreateHttpClient("127.0.0.1:4523/game-duration/v1", false);
    NewGameSession();
    InitEvents();
    InitHeatBeats();
    InitReportThread();
    InitRequest();
}

void DurationStatistics::InitEvents() {
    event_listener.listen([this](const events::User& new_user) { OnNewUser(new_user.user); });
    event_listener.listen([this](const events::Foreground& ignore) { OnForeground(); });
    event_listener.listen([this](const events::Background& ignore) { OnBackground(); });
    event_listener.listen([this](const events::GameStart& game_start) {
        if (game_start.game) {
            GameStart(*game_start.game);
        }
    });
}

void DurationStatistics::InitHeatBeats() {
    auto& timer = Runtime::Get().Timer();
    local_heat_beat = Event::Create([&timer, this](Duration later, std::uintptr_t user_data) {
        if (foreground && !game_id.empty()) {
            DurEvent event{.action = HEAT_BEAT,
                           .user_id = user_id,
                           .game_id = game_id,
                           .game_pkg = game_pkg,
                           .session = game_session,
                           .timestamp = Timestamp()};
            persistence->AddOrMergeEvent(event);
        }
        timer.PostEvent(local_heat_beat, local_heat_beat_interval);
    });
    online_heat_beat = Event::Create([&timer, this](Duration later, std::uintptr_t user_data) {
        if (foreground && !game_id.empty()) {
            DurEvent event{.action = HEAT_BEAT,
                           .user_id = user_id,
                           .game_id = game_id,
                           .game_pkg = game_pkg,
                           .session = game_session,
                           .timestamp = Timestamp()};
            report_queue.Put(event);
        }
        timer.PostEvent(online_heat_beat, online_tick_interval);
    });
    timer.PostEvent(local_heat_beat, local_heat_beat_interval);
    timer.PostEvent(online_heat_beat, online_tick_interval);
}

void DurationStatistics::InitReportThread() {
    constexpr auto max_report_events = 32;
    report_thread = std::make_unique<std::thread>([this]() {
        SetCurrentThreadName("EventReporter");
        while (running) {
            auto events = report_queue.Take(max_report_events);
            auto now = Runtime::Get().Timer().TimeEpoch();

            // Do report
            bool report_success;
            do {
                std::list<ReportContent> reports{};
                bool has_heat_beats{false};
                for (auto& event : events) {
                    if (event.action == HEAT_BEAT && event.id != 0) {
                        // local heart beat
                        continue;
                    }
                    if (event.action != HEAT_BEAT) {
                        reports.emplace_back(event);
                        continue;
                    }
                    if (has_heat_beats || !foreground) {
                        continue;
                    }
                    if (now - latest_online_report < online_tick_interval) {
                        continue;
                    }
                    event.timestamp = Timestamp();
                    has_heat_beats = true;
                }
                report_success = SyncAwait(http_client->PostAsync<ReportResult>(
                                                   "reporting", {}, {}, reports))
                                         .has_value();
                if (report_success) {
                    persistence->Delete(events);
                }
            } while (!report_success && running);
        }
    });
    // Old report
    auto pending_events = persistence->GetEvents();
    if (!pending_events.empty()) {
        report_queue.Put(pending_events);
    }
}

void DurationStatistics::InitRequest() {
    request_config = Event::Create([this](Duration later, std::uintptr_t user_data) {
        constexpr auto config_retry = Ms(30 * 1000);
        auto config_retry_times = 3;
        WebPath function{"reporting-config"};
        auto result = SyncAwait(
                http_client->GetAsync<ReportConfig>(function / game_id, {}, {}));
        if (result) {
            online_config = *result;
        } else {
            if (config_retry_times-- > 0) {
                Runtime::Get().Timer().PostEvent(request_config, config_retry);
            }
        }
    }, "RequestConfig");
}

void DurationStatistics::NewGameSession() {
    // pre game session
    auto latest_event = persistence->GetLatestEvent();
    if (latest_event && latest_event->action != GAME_END) {
        // new game end event
        auto new_event = *latest_event;
        new_event.action = GAME_END;
        persistence->AddOrMergeEvent(new_event);
    }

    // new game
    game_session = CreateUUID();
}

void DurationStatistics::GameStart(Game& game) {
    game_id = game.GetGameID();
    game_pkg = game.GetPackageName();
    DurEvent event{.action = GAME_START,
                   .user_id = user_id,
                   .game_id = game_id,
                   .game_pkg = game_pkg,
                   .session = game_session,
                   .timestamp = Timestamp()};
    persistence->AddOrMergeEvent(event);
    report_queue.Put(event);
}

void DurationStatistics::OnBackground() {
    foreground = false;
    DurEvent event{.action = GAME_BACKGROUND,
                   .user_id = user_id,
                   .game_id = game_id,
                   .game_pkg = game_pkg,
                   .session = game_session,
                   .timestamp = Timestamp()};
    persistence->AddOrMergeEvent(event);
    report_queue.Put(event);
}

void DurationStatistics::OnForeground() {
    foreground = true;
    DurEvent event{.action = GAME_FOREGROUND,
                   .user_id = user_id,
                   .game_id = game_id,
                   .game_pkg = game_pkg,
                   .session = game_session,
                   .timestamp = Timestamp()};
    persistence->AddOrMergeEvent(event);
    report_queue.Put(event);
}

void DurationStatistics::OnNewUser(const std::shared_ptr<TDSUser>& user) {
    if (user) {
        // login
        user_id = user->GetUserId();
        online_tick_interval = user->ContainTapInfo() ? online_heat_beat_interval : online_heat_beat_interval_no_tap;
        DurEvent event{.action = USER_LOGIN,
                       .user_id = user_id,
                       .game_id = game_id,
                       .game_pkg = game_pkg,
                       .session = game_session,
                       .timestamp = Timestamp()};
        persistence->AddOrMergeEvent(event);
        report_queue.Put(event);
        Runtime::Get().Timer().PostEvent(request_config);
    } else {
        // logout
        user_id = "";
        online_tick_interval = online_heat_beat_interval_no_tap;
        DurEvent event{.action = USER_LOGOUT,
                       .user_id = user_id,
                       .game_id = game_id,
                       .game_pkg = game_pkg,
                       .session = game_session,
                       .timestamp = Timestamp()};
        persistence->AddOrMergeEvent(event);
        report_queue.Put(event);
    }
}

u64 DurationStatistics::Timestamp() {
    std::time_t time_stamp = std::time(nullptr);
    return time_stamp;
}

DurationStatistics::~DurationStatistics() {
    if (report_thread) {
        running = false;
        report_queue.Put(DurEvent{});
        report_thread->join();
    }
}

}  // namespace tapsdk::duration
