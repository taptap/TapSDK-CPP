//
// Created by 甘尧 on 2023/7/7.
//

#include "base/thread.h"
#include "timer.h"

namespace tapsdk {

std::shared_ptr<Event> Event::Create(EventCall cb, std::string name) {
    auto res = std::make_shared<Event>();
    res->callback_ = std::move(cb);
    res->name_ = std::move(name);
    return std::move(res);
}

void Event::Invoke(Duration later, std::uintptr_t user_data) {
    if (active_) {
        callback_(later, user_data);
    }
}

void CoreTimer::Start() { timer_thread = std::make_unique<std::thread>(TimerThreadEntry, this); }

void CoreTimer::Resume() {
    paused = false;
    pause_event.Set();
}

void CoreTimer::Pause() {
    paused = true;
    pause_event.Set();
}

CoreTimer::Handle CoreTimer::PostEvent(const std::shared_ptr<Event>& event,
                                       Duration duration,
                                       std::uintptr_t user_data) {
    Handle handle;
    {
        std::scoped_lock scope{basic_lock};
        event->Active();
        const auto timeout = std::chrono::duration_cast<Ns>(duration) + Ns(TimeNs());

        handle = event_fifo_id++;
        msg_queue.emplace_back(event, timeout, handle, user_data);

        std::push_heap(msg_queue.begin(), msg_queue.end(), std::greater<>());
    }
    host_event.Set();
    return handle;
}

void CoreTimer::RemoveMessage(CoreTimer::Handle event) {
    std::scoped_lock scope{basic_lock};
    const auto itr = std::remove_if(msg_queue.begin(), msg_queue.end(), [&](const Message& m) {
        return m.handle == event;
    });

    if (itr != msg_queue.end()) {
        msg_queue.erase(itr, msg_queue.end());
        std::make_heap(msg_queue.begin(), msg_queue.end(), std::greater<>());
    }
}

void CoreTimer::RemoveEvent(const std::shared_ptr<Event>& event) {
    std::scoped_lock scope{basic_lock};
    event->InActive();
    const auto itr = std::remove_if(msg_queue.begin(), msg_queue.end(), [&](const Message& m) {
        return m.event.lock().get() == event.get();
    });

    if (itr != msg_queue.end()) {
        msg_queue.erase(itr, msg_queue.end());
        std::make_heap(msg_queue.begin(), msg_queue.end(), std::greater<>());
    }
}

void CoreTimer::TimerThreadEntry(void* timer) {
    SetCurrentThreadName("CoreTimer");
    reinterpret_cast<CoreTimer*>(timer)->Looper();
}

void CoreTimer::Looper() {
    while (!shutting_down) {
        while (!paused) {
            paused_set = false;
            const auto next_time = Advance();
            if (next_time) {
                if (*next_time > 0) {
                    auto next_time_ns = Ns(*next_time);
                    host_event.WaitFor(next_time_ns);
                }
            } else {
                wait_set = true;
                host_event.Wait();
            }
            wait_set = false;
        }
        paused_set = true;
        pause_event.Wait();
    }
}

std::optional<s64> CoreTimer::Advance() {
    std::scoped_lock lock{advance_lock, basic_lock};
    global_timer = TimeNs();

    while (!msg_queue.empty() && msg_queue.front().when <= Ns(global_timer)) {
        Message msg = std::move(msg_queue.front());
        std::pop_heap(msg_queue.begin(), msg_queue.end(), std::greater<>());
        msg_queue.pop_back();
        basic_lock.unlock();

        if (const auto event_type{msg.event.lock()}) {
            event_type->Invoke(Ns(static_cast<s64>(global_timer)) - msg.when, msg.user_data);
        }

        basic_lock.lock();
        global_timer = TimeNs();
    }

    if (!msg_queue.empty()) {
        const s64 next_time = msg_queue.front().when.count() - global_timer;
        return next_time;
    } else {
        return std::nullopt;
    }
}

std::chrono::nanoseconds CoreTimer::TimeEpoch() {
    return std::chrono::high_resolution_clock::now().time_since_epoch();
}

u64 CoreTimer::TimeNs() {
    return TimeEpoch().count();
}

void CoreTimer::SetOnlineTime(Ms ms) {
    auto local_now = std::chrono::system_clock::now().time_since_epoch();
    online_time = ms;
    time_ep_since_online = std::chrono::duration_cast<Ms>(local_now);
    online_time_set = true;
}

Ms CoreTimer::OnlineTimestamp() {
    auto local_now_ms = std::chrono::duration_cast<Ms>(std::chrono::system_clock::now().time_since_epoch());
    if (online_time_set) {
        return local_now_ms - time_ep_since_online + online_time;
    } else {
        return local_now_ms;
    }
}

CoreTimer::~CoreTimer() {
    shutting_down = true;
    paused = true;
    shutting_down = true;
    pause_event.Set();
    host_event.Set();
    if (timer_thread) {
        timer_thread->join();
    }
    timer_thread.reset();
    has_started = false;
}

}  // namespace tapsdk
