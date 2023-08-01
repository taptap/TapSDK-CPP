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
            DurEvent event{.action = HEAT_BEAT,
                           .tap_user = tap_user,
                           .user_id = user_id,
                           .game_id = game_id,
                           .game_pkg = game_pkg,
                           .device_id = device_id,
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
    constexpr auto max_report_events = 20;
    report_thread = std::make_unique<std::thread>([this]() {
        SetCurrentThreadName("EventReporter");
        u64 latest_online_report{};
        u64 latest_online_heat_beats{};
        while (running) {
            auto events = report_queue.Take(max_report_events);
            auto now = Timestamp();

            // Do report
            bool report_success;
            do {
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
                    if (now - latest_online_report < online_tick_interval.count()) {
                        continue;
                    }
                    event.last_timestamp = latest_online_heat_beats;
                    latest_online_report = now;
                    event.timestamp = now;
                    has_heat_beats = true;
                    reports.emplace_back(event);
                }
                if (reports.empty()) {
                    continue;
                }
                report_success =
                        http_client->PostSync<ReportResult>("statistics", {}, {}, reports).has_value();
                if (report_success) {
                    latest_online_heat_beats = now;
                    persistence->Delete(events);
                } else {
                    report_retry_event.WaitFor(retry_ms);
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
    request_config = Event::Create(
            [this](Duration later, std::uintptr_t user_data) {
                constexpr auto config_retry = Ms(3 * 1000);
                auto config_retry_times = 3;
                WebPath function{"config"};
                auto result = http_client->GetSync<ReportConfig>(function / game_id, {}, {});
                if (result) {
                    report_config = *result;
                    Runtime::Get().Timer().SetOnlineTime(Ms(report_config->ServerTimestamp() * 1000));
                    online_heat_beat_interval = Ms(report_config->TapFrequency() * 1000);
                    online_heat_beat_interval_no_tap = Ms(report_config->NoTapFrequency() * 1000);
                    online_tick_interval =
                            tap_user ? online_heat_beat_interval : online_heat_beat_interval_no_tap;
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
                           .timestamp = session.last_beats};
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
    persistence->UpdateSession(local_session);
    Runtime::Get().Timer().PostEvent(request_config);
}

void DurationStatistics::OnBackground() {
    ASSERT_MSG(!game_id.empty(), "Set current game first!");
    foreground = false;
    DurEvent event{.action = GAME_BACKGROUND,
                   .tap_user = tap_user,
                   .user_id = user_id,
                   .game_id = game_id,
                   .game_pkg = game_pkg,
                   .device_id = device_id,
                   .session = game_session,
                   .timestamp = Timestamp()};
    persistence->AddOrMergeEvent(event);
    report_queue.Put(event);
}

void DurationStatistics::OnForeground() {
    ASSERT_MSG(!game_id.empty(), "Set current game first!");
    foreground = true;
    DurEvent event{.action = GAME_FOREGROUND,
                   .tap_user = tap_user,
                   .user_id = user_id,
                   .game_id = game_id,
                   .game_pkg = game_pkg,
                   .device_id = device_id,
                   .session = game_session,
                   .timestamp = Timestamp()};
    persistence->AddOrMergeEvent(event);
    report_queue.Put(event);
}

void DurationStatistics::OnNewUser(const std::shared_ptr<TDSUser>& user) {
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
                           .timestamp = Timestamp()};
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
                       .timestamp = Timestamp()};
        persistence->AddOrMergeEvent(event);
        report_queue.Put(event);
    }
}

u64 DurationStatistics::Timestamp() { return Runtime::Get().Timer().OnlineTimestamp().count(); }

DurationStatistics::~DurationStatistics() {
    if (report_thread) {
        running = false;
        report_queue.Stop();
        report_retry_event.Set();
        report_thread->join();
    }
}

}  // namespace tapsdk::duration
