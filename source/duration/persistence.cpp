//
// Created by 甘尧 on 2023/7/10.
//

#include "base/logging.h"
#include "persistence.h"

namespace tapsdk::duration {

DurPersistence::DurPersistence(const fs::path& cache_dir)
        : storage{InitDB(cache_dir / "tap_dur.sqlite")} {
    storage.sync_schema();
}

void DurPersistence::AddOrMergeEvent(DurEvent& event) {
    std::unique_lock guard(lock);
    auto latest = storage.get_all<DurEvent>(order_by(&DurEvent::id).desc(), limit(1));
    if (!latest.empty()) {
        auto& old = latest.back();
        // Merge
        if (event.action == old.action && old.session == event.session) {
            event.id = old.id;
            storage.update(event);
        } else {
            event.id = storage.insert(event);
        }
    } else {
        // Append
        event.id = storage.insert(event);
    }
}

void DurPersistence::Update(DurEvent& event) {
    std::unique_lock guard(lock);
    storage.update(event);
}

void DurPersistence::Delete(DurEvent& event) {
    std::unique_lock guard(lock);
    storage.remove<DurEvent>(event.id);
}

void DurPersistence::Delete(std::vector<DurEvent>& events) {
    std::unique_lock guard(lock);
    for (DurEvent& event : events) {
        if (event.id) {
            storage.remove<DurEvent>(event.id);
        }
    }
}

std::optional<DurEvent> DurPersistence::GetLatestEvent() {
    std::shared_lock guard(lock);
    auto latest = storage.get_all<DurEvent>(order_by(&DurEvent::id).desc(), limit(1));
    if (!latest.empty()) {
        auto& old = latest.back();
        return old;
    } else {
        return std::nullopt;
    }
}

std::vector<DurEvent> DurPersistence::GetEvents() {
    std::shared_lock guard(lock);
    return storage.get_all<DurEvent>(order_by(&DurEvent::id).asc());
}

void DurPersistence::UpdateSession(GameSession &session) {
    std::unique_lock guard(lock);
    if (session.id == 0) {
        session.id = storage.insert(session);
    } else {
        storage.update(session);
    }
}

void DurPersistence::UpdateConfig(ReportConfig& config) {
    std::unique_lock guard(lock);
    if (config.id == 0) {
        config.id = storage.insert(config);
    } else {
        storage.update(config);
    }
}

std::optional<ReportConfig> DurPersistence::GetLatestConfig() {
    std::shared_lock guard(lock);
    auto latest = storage.get_all<ReportConfig>(order_by(&ReportConfig::id).desc(), limit(1));
    if (!latest.empty()) {
        auto& old = latest.back();
        return old;
    } else {
        return std::nullopt;
    }
}

std::optional<GameSession> DurPersistence::GetLatestSession() {
    std::shared_lock guard(lock);
    auto latest = storage.get_all<GameSession>(order_by(&GameSession::id).desc(), limit(1));
    if (!latest.empty()) {
        auto& old = latest.back();
        return old;
    } else {
        return std::nullopt;
    }
}

}  // namespace tapsdk::duration
