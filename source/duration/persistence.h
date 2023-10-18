//
// Created by 甘尧 on 2023/7/10.
//

#pragma once

#include <memory>
#include <shared_mutex>
#include "duration/dur_model.h"
#include "ghc/fs_std_select.hpp"
#include "sqlite_orm/sqlite_orm.h"

namespace tapsdk::duration {
using namespace sqlite_orm;

inline auto InitDB(const fs::path& path) {
    auto event_table = make_table<DurEvent>("dur_event",
                                            make_column("id", &DurEvent::id, primary_key().autoincrement()),
                                            make_column("action", &DurEvent::action),
                                            make_column("session", &DurEvent::session),
                                            make_column("timestamp", &DurEvent::timestamp),
                                            make_column("tap_user", &DurEvent::tap_user),
                                            make_column("user_id", &DurEvent::user_id),
                                            make_column("game_id", &DurEvent::game_id),
                                            make_column("game_pkg", &DurEvent::game_pkg),
                                            make_column("device_id", &DurEvent::device_id),
                                            make_column("last_timestamp", &DurEvent::last_timestamp));
    auto game_session_table = make_table<GameSession>("game_session",
                                            make_column("id", &GameSession::id, primary_key().autoincrement()),
                                            make_column("session", &GameSession::session),
                                            make_column("game_start", &GameSession::game_start),
                                            make_column("last_beats", &GameSession::last_beats),
                                            make_column("tap_user", &GameSession::tap_user),
                                            make_column("user_id", &GameSession::user_id),
                                            make_column("game_id", &GameSession::game_id),
                                            make_column("game_pkg", &GameSession::game_pkg),
                                            make_column("last_timestamp", &GameSession::last_timestamp));
    auto config_table = make_table<ReportConfig>("report_config",
                                            make_column("id", &ReportConfig::id, primary_key().autoincrement()),
                                            make_column("enable", &ReportConfig::enable),
                                            make_column("no_tap_enable", &ReportConfig::no_tap_enable),
                                            make_column("tap_frequency", &ReportConfig::tap_frequency),
                                            make_column("no_tap_frequency", &ReportConfig::no_tap_frequency));
    return make_storage(path.string(), event_table, game_session_table, config_table);
}

using Storage = decltype(InitDB(""));

class DurPersistence {
public:
    explicit DurPersistence(const fs::path& cache_dir);

    void AddOrMergeEvent(DurEvent &event);

    void Update(DurEvent &event);

    void Delete(DurEvent &event);

    void Delete(std::vector<DurEvent> &events);

    void UpdateSession(GameSession &session);

    void UpdateConfig(ReportConfig &config);

    std::optional<GameSession> GetLatestSession();

    std::optional<ReportConfig> GetLatestConfig();

    std::optional<DurEvent> GetLatestEvent();

    std::vector<DurEvent> GetEvents();

private:
    std::shared_mutex lock;
    Storage storage;
};

}  // namespace tapsdk::duration
