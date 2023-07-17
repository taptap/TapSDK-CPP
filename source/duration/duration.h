//
// Created by 甘尧 on 2023/7/7.
//

#pragma once

#include "base/event.h"
#include "base/timer.h"
#include "core/runtime.h"
#include "duration/persistence.h"
#include "net/network.h"
#include "base/blocking_queue.h"

namespace tapsdk::duration {

constexpr auto local_heat_beat_interval = Ms(5 * 1000);
constexpr auto online_heat_beat_interval = Ms(2 * 60 * 1000);
constexpr auto online_heat_beat_interval_no_tap = Ms(5 * 60 * 1000);

class DurationStatistics {
public:
    void Init();

    virtual ~DurationStatistics();

private:

    void InitRequest();

    void InitEvents();

    void InitHeatBeats();

    void InitReportThread();

    void NewGameSession();

    void GameStart(Game &game);

    void OnForeground();

    void OnBackground();

    void OnNewUser(const std::shared_ptr<TDSUser> &user);

    u64 Timestamp();

    std::atomic_bool running{true};
    std::atomic_bool foreground{true};
    std::shared_mutex event_lock;
    std::shared_ptr<Event> request_config;
    std::shared_ptr<Event> local_heat_beat;
    std::shared_ptr<Event> online_heat_beat;
    std::shared_ptr<ReportConfig> online_config;
    Duration online_tick_interval{online_heat_beat_interval};
    std::unique_ptr<DurPersistence> persistence{};
    dexode::EventBus::Listener event_listener{Runtime::Get().GetEventBus()};
    std::unique_ptr<net::TapHttpClient> http_client{};
    std::unique_ptr<std::thread> report_thread{};
    BlockingQueue<DurEvent> report_queue{UINT32_MAX};
    std::string user_id;
    std::string game_id;
    std::string game_pkg;
    std::string game_session;
    std::chrono::nanoseconds latest_online_report{};
};

}  // namespace tapsdk::duration
