//
// Created by 甘尧 on 2023/7/7.
//

#pragma once

#include <atomic>
#include <condition_variable>
#include "base/blocking_queue.h"
#include "base/event.h"
#include "base/timer.h"
#include "core/runtime.h"
#include "duration/persistence.h"
#include "net/network.h"
#include "sdk/platform.h"

namespace tapsdk::duration {

constexpr auto retry_ms = Ms(5 * 1000);
constexpr auto local_heat_beat_interval = Ms(5 * 1000);

class DurationStatistics {
public:
    void Init(Region region);

    virtual ~DurationStatistics();

private:
    void InitRequest();

    void InitEvents();

    void InitHeatBeats();

    void InitHistory();

    void InitReportThread();

    void NewGameSession();

    void GameStart(Game& game);

    void OnForeground();

    void OnBackground();

    void OnNewUser(const std::shared_ptr<TDSUser>& user);

    void RefreshConfig(ReportConfig& config, bool net);

    u64 Timestamp();

    void Stop();

    std::atomic_bool running{false};
    std::atomic_bool foreground{true};
    GameSession local_session{};
    std::shared_mutex event_lock;
    std::shared_mutex config_lock;
    std::shared_ptr<Event> request_config;
    std::shared_ptr<Event> local_heat_beat;
    std::shared_ptr<Event> online_heat_beat;
    ReportConfig report_config{};
    Duration online_heat_beat_interval = Ms(report_config.tap_frequency * 1000);
    Duration online_heat_beat_interval_no_tap = Ms(report_config.no_tap_frequency * 1000);
    Duration online_tick_interval{online_heat_beat_interval_no_tap};
    std::unique_ptr<DurPersistence> persistence{};
    dexode::EventBus::Listener event_listener{Runtime::Get().GetEventBus()};
    std::unique_ptr<net::TapHttpClient> http_client{};
    std::unique_ptr<std::thread> report_thread{};
    BlockingQueue<DurEvent> report_queue{UINT32_MAX};
    HostEvent report_retry_event{};
    bool tap_user;
    platform::DeviceType dev_type;
    std::string device_id;
    std::string user_id;
    std::string game_id;
    std::string game_pkg;
    std::string game_session;
};

}  // namespace tapsdk::duration
