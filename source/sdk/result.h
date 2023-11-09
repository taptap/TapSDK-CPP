#pragma once

#include <chrono>
#include <condition_variable>
#include <memory>
#include <mutex>
#include <string>
#include <utility>
#include "config.h"

namespace tapsdk {

class TDS_EXPORT NonCopyAndMove {
public:
    NonCopyAndMove(const NonCopyAndMove&) = delete;
    NonCopyAndMove(NonCopyAndMove&&) = delete;
    virtual ~NonCopyAndMove() = default;
    NonCopyAndMove& operator=(const NonCopyAndMove&) = delete;
    NonCopyAndMove& operator=(NonCopyAndMove&&) = delete;

protected:
    NonCopyAndMove() = default;
};

enum ErrorCode { NO_ERR = 0, ERR_TIMEOUT = 1 };

struct TDS_EXPORT Error {
    int code = NO_ERR;
    std::string msg;
};

template <typename T = void> class TDS_EXPORT Result {
public:
    explicit Result() : error{NO_ERR} {}

    explicit Result(const std::shared_ptr<T>& v) : value{v} {}

    explicit Result(Error e) : error{std::move(e)} {}

    std::shared_ptr<T> GetValue() const { return value; }

    Error GetError() const { return error; }

    bool Success() const { return error.code != NO_ERR; }

private:
    std::shared_ptr<T> value{};
    Error error{};
};

template <typename T> class Future;
template <typename T> class TDS_EXPORT FutureCallback {
public:
    virtual void Callback(const Result<T>& result){};
};

template <typename T> class Promise : NonCopyAndMove {
    friend class Future<T>;

private:
    std::mutex lock{};
    std::condition_variable cond_var{};
    bool retrieved{false};
    Result<T> result{Error{ERR_TIMEOUT}};
    FutureCallback<T>* cb{};
};

template <typename T> class TDS_EXPORT Future {
public:
    Future() : promise{std::make_shared<Promise<T>>()} {}

    void AWait(FutureCallback<T>& callback) {
        std::unique_lock guard(promise->lock);
        if (promise->retrieved) {
            guard.unlock();
            callback.Callback(promise->result);
        } else {
            promise->cb = &callback;
        }
    }

    Result<T>& Get() {
        std::unique_lock guard(promise->lock);
        while (!promise->retrieved) {
            promise->cond_var.wait(guard);
        }
        return promise->result;
    }

    Result<T>& Get(uint64_t timeout_ms) {
        std::unique_lock guard(promise->lock);
        bool timeout = false;
        while (!promise->retrieved || timeout) {
            timeout = promise->cond_var.wait_for(guard, std::chrono::milliseconds(timeout_ms)) ==
                      std::cv_status::timeout;
        }
        return promise->result;
    }

    const Result<T>& operator*() const& { return Get(); }
    Result<T>& operator*() & { return Get(); }
    const Result<T>&& operator*() const&& { return std::move(Get()); }
    Result<T>&& operator*() && { return std::move(Get()); }

    void Set(const Result<T>& res) const {
        std::unique_lock guard(promise->lock);
        if (promise->retrieved) {
            return;
        }
        promise->result = res;
        promise->retrieved = true;
        promise->cond_var.notify_all();
        if (promise->cb) {
            guard.unlock();
            promise->cb->Callback(promise->result);
        }
    }

private:
    std::shared_ptr<Promise<T>> promise;
};

}  // namespace tapsdk
