//
// Created by 甘尧 on 2023/7/7.
//

#pragma once

#include <memory>
#include <mutex>
#include <thread>
#include <optional>
#include <utility>
#include <vector>
#include <chrono>
#include "event.h"
#include "types.h"

namespace tapsdk {

using Duration = std::chrono::steady_clock::duration;
using EventCall = std::function<void(Duration later, std::uintptr_t user_data)>;

using Ns = std::chrono::nanoseconds;
using Ms = std::chrono::milliseconds;

class Event {
public:
    static std::shared_ptr<Event> Create(EventCall cb, std::string name = "anom");

    void Invoke(Duration later, std::uintptr_t user_data);

    void Active() { active_ = true; }

    void InActive() { active_ = false; }

    bool IsActive() { return active_; }

private:
    std::string name_;
    EventCall callback_;
    bool active_{true};
};

class CoreTimer {
public:
    using Handle = u32;

    struct Message {
        std::weak_ptr<Event> event{};
        Ns when{};
        std::uintptr_t user_data{};
        Handle handle{};

        explicit Message() = default;

        explicit Message(const std::shared_ptr<Event>& event,
                         const Ns& when,
                         Handle id,
                         std::uintptr_t user_data)
                : event{event}, when{when}, handle{id}, user_data{user_data} {};

        [[nodiscard]] bool Null() const { return event.lock() == nullptr; }

        friend bool operator>(const Message& left, const Message& right) {
            return std::tie(left.when, left.handle) > std::tie(right.when, right.handle);
        }

        friend bool operator<(const Message& left, const Message& right) {
            return std::tie(left.when, left.handle) < std::tie(right.when, right.handle);
        }
    };

    explicit CoreTimer() = default;

    virtual ~CoreTimer();

    void Start();

    void Resume();

    void Pause();

    Handle PostEvent(const std::shared_ptr<Event>& event,
                     Duration duration = {},
                     std::uintptr_t user_data = {});

    void RemoveMessage(Handle event);

    void RemoveEvent(const std::shared_ptr<Event>& event);

    u64 TimeNs();

    std::chrono::nanoseconds TimeEpoch();

private:
    static void TimerThreadEntry(void* timer);

    void Looper();

    std::optional<s64> Advance();

    std::unique_ptr<std::thread> timer_thread;
    std::vector<Message> msg_queue;
    u32 event_fifo_id = 0;
    std::shared_ptr<Message> ev_lost{};
    HostEvent host_event{};
    HostEvent pause_event{};
    std::mutex basic_lock{};
    std::mutex advance_lock{};

    u64 global_timer = 0;

    std::atomic_bool paused{};
    std::atomic_bool paused_set{};
    std::atomic_bool wait_set{};
    std::atomic_bool shutting_down{};
    std::atomic_bool has_started{};
};

}  // namespace tapsdk
