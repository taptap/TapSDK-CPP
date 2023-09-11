#pragma once

#include <condition_variable>
#include <functional>
#include <memory>
#include <mutex>
#include <string>

namespace tapsdk {

class NonCopyAndMove {
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

struct Error {
    ErrorCode code{NO_ERR};
    std::string msg;
};

template <typename T> class Result {
public:
    Result() = default;

    Result(const std::shared_ptr<T>& v) : value{v} {}

    Result(const Error& e) : error{e} {}

    std::shared_ptr<T> GetValue() {
        return value;
    }

    Error GetError() {
        return error;
    }

private:
    std::shared_ptr<T> value{};
    Error error{};
};

template <typename T> class Future;
template <typename T> class FutureCallback {
public:
    virtual void Callback(const Result<T> &result) {};
};

template <typename T> class Promise : NonCopyAndMove {
    friend class Future<T>;
private:
    std::mutex lock{};
    std::condition_variable cond_var{};
    bool retrieved{false};
    Result<T> result{};
    FutureCallback<T> *cb{};
};

template <typename T> class Future {
public:

    Future() : promise{std::make_shared<Promise<T>>()} {}

    void AWait(FutureCallback<T> &callback) {
        std::unique_lock guard(promise->lock);
        if (promise->retrieved) {
            guard.unlock();
            callback.Callback(promise->result);
        } else {
            promise->cb = &callback;
        }
    }

    Result<T> Get() {
        std::unique_lock guard(promise->lock);
        while (!promise->retrieved) {
            promise->cond_var.wait(guard);
        }
        return promise->result;
    }

    void Set(const Result<T>& res) {
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
