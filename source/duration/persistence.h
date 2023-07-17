//
// Created by 甘尧 on 2023/7/10.
//

#pragma once

#include <filesystem>
#include <memory>
#include <shared_mutex>
#include "duration/model.h"
#include "sqlite_orm/sqlite_orm.h"

namespace tapsdk::duration {
using namespace sqlite_orm;

inline auto InitDB(const std::filesystem::path& path) {
    auto event_table = make_table<DurEvent>("dur_event",
                                            make_column("id", &DurEvent::id, primary_key().autoincrement()),
                                            make_column("action", &DurEvent::action),
                                            make_column("session", &DurEvent::session),
                                            make_column("timestamp", &DurEvent::timestamp),
                                            make_column("user_id", &DurEvent::user_id),
                                            make_column("game_id", &DurEvent::game_id),
                                            make_column("game_pkg", &DurEvent::game_pkg));
    return make_storage(path.string(), event_table);
}

using Storage = decltype(InitDB(""));

class DurPersistence {
public:
    explicit DurPersistence(const std::filesystem::path& cache_dir);

    void AddOrMergeEvent(DurEvent &event);

    void Update(DurEvent &event);

    void Delete(DurEvent &event);

    void Delete(std::vector<DurEvent> &events);

    std::optional<DurEvent> GetLatestEvent();

    std::vector<DurEvent> GetEvents();
    
private:
    std::shared_mutex lock;
    Storage storage;
};

}  // namespace tapsdk::duration
