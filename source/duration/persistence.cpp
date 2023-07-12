//
// Created by 甘尧 on 2023/7/10.
//

#include "base/logging.h"
#include "persistence.h"

namespace tapsdk::duration {

DurPersistence::DurPersistence(const std::filesystem::path& cache_dir)
        : storage{InitDB(cache_dir / "tap_dur.sqlite")} {
    storage.sync_schema();
}

void DurPersistence::AddOrMergeEvent(DurEvent& event) {
    std::unique_lock guard(lock);
    auto latest = storage.get_all<DurEvent>(order_by(&DurEvent::id).desc(), limit(1));
    if (!latest.empty()) {
        auto &old = latest.back();
        // Merge
    } else {
        // Append
        storage.insert(event);
    }
}

}  // namespace tapsdk::duration
