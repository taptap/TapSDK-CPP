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
    device_id = cur_device->GetDeviceID();
    persistence = std::make_unique<DurPersistence>(cur_device->GetCacheDir());
    http_client = net::CreateHttpClient("tds-activity-collector.xdrnd.cn/report/v1", true);
    NewGameSession();
    InitEvents();
    InitHeatBeats();
    InitHistory();
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
            local_session.last_beats = Timestamp();
            persistence->UpdateSession(local_session);
        }
        timer.PostEvent(local_heat_beat, local_heat_beat_interval);
    });
    online_heat_beat = Event::Create([&timer, this](Duration later, std::uintptr_t user_data) {
        if (foreground && !game_id.empty()) {
            std::unique_lock guard(event_lock);
            DurEvent event{.action = HEAT_BEAT,
                           .tap_user = tap_user,
                           .user_id = user_id,
                           .game_id = game_id,
                           .game_pkg = game_pkg,
                           .device_id = device_id,
                           .session = game_session,
                           .timestamp = Timestamp(),
                           .last_timestamp = local_session.last_timestamp};
            report_queue.Put(event);
        }
        timer.PostEvent(online_heat_beat, online_tick_interval);
    });
    timer.PostEvent(local_heat_beat, local_heat_beat_interval);
    timer.PostEvent(online_heat_beat, online_tick_interval);
}

void DurationStatistics::InitHistory() {
    // Config cache
    auto config = persistence->GetLatestConfig();
    if (config) {
        RefreshConfig(*config, false);
    }
    // Old report
    auto pending_events = persistence->GetEvents();
    if (!pending_events.empty()) {
        report_queue.Put(pending_events);
    }
}

void DurationStatistics::InitReportThread() {
    constexpr auto max_report_events = 20;
    if (running) return;
    if (!report_config.enable) return;
    running = true;
    report_thread = std::make_unique<std::thread>([this]() {
        SetCurrentThreadName("EventReporter");
        while (running) {
            auto events = report_queue.Take(max_report_events);

            // Do report
            bool report_success;
            do {
                auto now = Timestamp();
                std::list<ReportContent> reports{};
                bool has_heat_beats{false};
                for (auto& event : events) {
                    if (event.action != HEAT_BEAT) {
                        reports.emplace_back(event);
                        continue;
                    }
                    if (has_heat_beats || !foreground) {
                        continue;
                    }
                    event.timestamp = now;
                    event.last_timestamp = local_session.last_timestamp;
                    has_heat_beats = true;
                    reports.emplace_back(event);
                }
                if (reports.empty()) {
                    continue;
                }
                auto report_result =
                        http_client->PostSync<ReportResult>("statistics", {}, {}, reports);
                report_success = report_result.has_value();
                if (report_success) {
                    persistence->Delete(events);
                    if (has_heat_beats) {
                        std::unique_lock guard(event_lock);
                        if (now > local_session.last_timestamp) {
                            local_session.last_timestamp = now;
                            persistence->UpdateSession(local_session);
                        }
                    }
                    LOG_DEBUG("ReportSuccess: {}", reports.size());
                } else {
                    LOG_ERROR("ReportFailed: code: {}, msg: {}", report_result.error().code, report_result.error().msg);
                    report_retry_event.WaitFor(retry_ms);
                }
            } while (!report_success && running);
        }
    });
}

void DurationStatistics::InitRequest() {
    running = true;
    request_config = Event::Create(
            [this](Duration later, std::uintptr_t user_data) {
                constexpr auto config_retry = Ms(3 * 1000);
                auto config_retry_times = 3;
                WebPath function{"config"};
                auto result = http_client->GetSync<ReportConfig>(function / game_id, {}, {});
                if (result) {
                    RefreshConfig(**result, true);
                } else {
                    if (config_retry_times-- > 0) {
                        Runtime::Get().Timer().PostEvent(request_config, config_retry);
                    }
                }
            },
            "RequestConfig");
}

void DurationStatistics::NewGameSession() {
    // pre game session
    auto latest_session = persistence->GetLatestSession();
    if (latest_session) {
        // new game end event
        auto session = *latest_session;
        DurEvent new_event{.action = GAME_END,
                           .tap_user = tap_user,
                           .user_id = session.user_id,
                           .game_id = session.game_id,
                           .game_pkg = session.game_pkg,
                           .device_id = device_id,
                           .session = session.session,
                           .timestamp = session.last_beats,
                           .last_timestamp = session.last_timestamp};
        persistence->AddOrMergeEvent(new_event);
        user_id = latest_session->user_id;
        tap_user = latest_session->tap_user;
        online_tick_interval =
                tap_user ? online_heat_beat_interval : online_heat_beat_interval_no_tap;
    }

    // new game
    game_session = CreateUUID();
    local_session.session = game_session;
    persistence->UpdateSession(local_session);
}

void DurationStatistics::GameStart(Game& game) {
    std::unique_lock guard(event_lock);
    game_id = game.GetGameID();
    game_pkg = game.GetPackageName();
    DurEvent event{.action = GAME_START,
                   .tap_user = tap_user,
                   .user_id = user_id,
                   .game_id = game_id,
                   .game_pkg = game_pkg,
                   .device_id = device_id,
                   .session = game_session,
                   .timestamp = Timestamp()};
    persistence->AddOrMergeEvent(event);
    report_queue.Put(event);
    local_session.game_id = game_id;
    local_session.game_pkg = game_pkg;
    local_session.game_start = event.timestamp;
    local_session.last_beats = event.timestamp;
    local_session.last_timestamp = event.timestamp;
    persistence->UpdateSession(local_session);
    Runtime::Get().Timer().PostEvent(request_config);
}

void DurationStatistics::OnBackground() {
    std::unique_lock guard(event_lock);
    ASSERT_MSG(!game_id.empty(), "Set current game first!");
    foreground = false;
    DurEvent event{.action = GAME_BACKGROUND,
                   .tap_user = tap_user,
                   .user_id = user_id,
                   .game_id = game_id,
                   .game_pkg = game_pkg,
                   .device_id = device_id,
                   .session = game_session,
                   .timestamp = Timestamp(),
                   .last_timestamp = local_session.last_timestamp};
    local_session.last_beats = event.timestamp;
    local_session.last_timestamp = event.timestamp;
    persistence->UpdateSession(local_session);
    persistence->AddOrMergeEvent(event);
    report_queue.Put(event);
}

void DurationStatistics::OnForeground() {
    std::unique_lock guard(event_lock);
    ASSERT_MSG(!game_id.empty(), "Set current game first!");
    foreground = true;
    DurEvent event{.action = GAME_FOREGROUND,
                   .tap_user = tap_user,
                   .user_id = user_id,
                   .game_id = game_id,
                   .game_pkg = game_pkg,
                   .device_id = device_id,
                   .session = game_session,
                   .timestamp = Timestamp(),
                   .last_timestamp = local_session.last_timestamp};
    local_session.last_beats = event.timestamp;
    local_session.last_timestamp = event.timestamp;
    persistence->UpdateSession(local_session);
    persistence->AddOrMergeEvent(event);
    report_queue.Put(event);
}

void DurationStatistics::OnNewUser(const std::shared_ptr<TDSUser>& user) {
    std::unique_lock guard(event_lock);
    if (user) {
        // login
        if (user_id != user->GetUserId()) {
            tap_user = user->ContainTapInfo();
            user_id = user->GetUserId();
            DurEvent event{.action = USER_LOGIN,
                           .tap_user = tap_user,
                           .user_id = user_id,
                           .game_id = game_id,
                           .game_pkg = game_pkg,
                           .device_id = device_id,
                           .session = game_session,
                           .timestamp = Timestamp(),
                           .last_timestamp = local_session.last_timestamp};
            local_session.last_beats = event.timestamp;
            local_session.last_timestamp = event.timestamp;
            persistence->UpdateSession(local_session);
            persistence->AddOrMergeEvent(event);
            report_queue.Put(event);
        }
        online_tick_interval =
                tap_user ? online_heat_beat_interval : online_heat_beat_interval_no_tap;
        local_session.user_id = user_id;
        local_session.tap_user = tap_user;
        persistence->UpdateSession(local_session);
    } else {
        // logout
        tap_user = false;
        user_id = "";
        online_tick_interval = online_heat_beat_interval_no_tap;
        DurEvent event{.action = USER_LOGOUT,
                       .tap_user = tap_user,
                       .user_id = user_id,
                       .game_id = game_id,
                       .game_pkg = game_pkg,
                       .device_id = device_id,
                       .session = game_session,
                       .timestamp = Timestamp(),
                       .last_timestamp = local_session.last_timestamp};
        local_session.last_beats = event.timestamp;
        local_session.last_timestamp = event.timestamp;
        local_session.user_id = user_id;
        local_session.tap_user = tap_user;
        persistence->UpdateSession(local_session);
        persistence->AddOrMergeEvent(event);
        report_queue.Put(event);
    }
}

void DurationStatistics::RefreshConfig(ReportConfig& config, bool net) {
    std::unique_lock guard(config_lock);
    report_config = config;
    online_heat_beat_interval = Ms(config.TapFrequency() * 1000);
    online_heat_beat_interval_no_tap = Ms(config.NoTapFrequency() * 1000);
    online_tick_interval =
            tap_user ? online_heat_beat_interval : online_heat_beat_interval_no_tap;
    if (net) {
        Runtime::Get().Timer().SetOnlineTime(Ms(config.ServerTimestamp() * 1000));
        if (config.Enabled()) {
            InitReportThread();
        } else {
            Stop();
        }
        persistence->UpdateConfig(report_config);
    }
}

u64 DurationStatistics::Timestamp() { return Runtime::Get().Timer().OnlineTimestamp().count(); }

void DurationStatistics::Stop() {
    if (running) {
        running = false;
        report_queue.Stop();
        report_retry_event.Set();
    }
}

DurationStatistics::~DurationStatistics() {
    if (report_thread) {
        Stop();
        report_thread->join();
    }
}

}  // namespace tapsdk::duration
