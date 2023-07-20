//
// Created by 甘尧 on 2023/7/7.
//

#pragma once

#include <mutex>
#include <chrono>
#include <atomic>
#include "types.h"

namespace tapsdk {

class HostEvent {
public:
    void Set() {
        std::lock_guard lk{mutex};
        if (!is_set) {
            is_set = true;
            cond_var.notify_one();
        }
    }

    void Wait() {
        std::unique_lock lk{mutex};
        cond_var.wait(lk, [&] { return is_set.load(); });
        is_set = false;
    }

    bool WaitFor(const std::chrono::nanoseconds& time) {
        std::unique_lock lk{mutex};
        if (!cond_var.wait_for(lk, time, [this] { return is_set.load(); })) return false;
        is_set = false;
        return true;
    }

    template <class Clock, class Duration>
    bool WaitUntil(const std::chrono::time_point<Clock, Duration>& time) {
        std::unique_lock lk{mutex};
        if (!cond_var.wait_until(lk, time, [this] { return is_set.load(); })) return false;
        is_set = false;
        return true;
    }

    void Reset() {
        std::unique_lock lk{mutex};
        is_set = false;
    }

private:
    std::condition_variable cond_var;
    std::mutex mutex;
    std::atomic<bool> is_set{false};
};

}  // namespace tapsdk
