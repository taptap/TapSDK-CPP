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
                                            make_column("user_id", &DurEvent::user_id));
    return make_storage(path.string(), event_table);
}

using Storage = decltype(InitDB(""));

class DurPersistence {
public:
    explicit DurPersistence(const std::filesystem::path& cache_dir);

    void AddOrMergeEvent(DurEvent event);

    std::optional<DurEvent> GetLatestEvent();
    
private:
    std::shared_mutex lock;
    Storage storage;
};

}  // namespace tapsdk::duration
