//
// Created by 甘尧 on 2023/7/5.
//

#pragma once

#include <list>
#include <memory>
#include <string>

#ifdef ANDROID

#include "externals/compat/concepts.h"

#else
#include <concepts>
#endif

#include "async_simple/coro/Lazy.h"
#include "async_simple/coro/SyncAwait.h"
#include "base/expected.h"
#include "base/types.h"
#include "nlohmann/json.hpp"

namespace tapsdk::net {

using namespace nlohmann;
constexpr auto http_timeout_ms = 10 * 1000;  // ms

enum HttpType { GET, POST };

using Json = nlohmann::json;
using Pair = std::pair<std::string, std::string>;
using OnReturn = const std::function<void(const Json& content)>&;
template <class T> using OnSuccess = const std::function<void(const std::shared_ptr<T>& content)>&;
using OnFailed = const std::function<void(int status, int code, const std::string& msg)>&;
using Params = std::initializer_list<Pair>;
using Headers = std::initializer_list<Pair>;

class ResultWrap {
public:
    explicit ResultWrap(const char* response);

    [[nodiscard]] int GetCode() const;

    [[nodiscard]] const std::string& GetMsg() const;

    [[nodiscard]] const Json& GetContent() const;

private:
    int code;
    std::string msg;
    Json content;
};

struct Error {
    int status;
    int code;
    std::string msg;
};

inline unexpected<Error> MakeError(int status, int code, const std::string& msg) {
    return unexpected(Error{status, code, msg});
}

template <class T> using Result = expected<T, Error>;
template <class T> using ResultAsync = async_simple::coro::Lazy<Result<T>>;

template <typename T>
concept JsonResult = requires(const Json& str) {
    { T(str) };
};

template <typename T>
concept JsonParam = requires(T t) {
    { t.ToJson() } -> std::convertible_to<Json>;
};

template <typename LazyType> inline auto SyncAwait(LazyType&& lazy) {
    return async_simple::coro::syncAwait(lazy);
}

class TapHttpClient {
public:
    TapHttpClient(const char* host, bool https) : host{host}, https{https} {};

    virtual ~TapHttpClient() = default;

    virtual void CommonHeader(const char* key, const char* value) = 0;

    virtual void CommonParam(const char* key, const char* value) = 0;

    virtual void RequestAsync(HttpType type,
                              const WebPath& path,
                              Headers headers,
                              Params params,
                              OnReturn success,
                              OnFailed failed) = 0;

    virtual ResultAsync<Json> RequestAsync(HttpType type,
                                           const WebPath& path,
                                           Headers headers,
                                           Params params,
                                           const Json& content = {}) = 0;

    template <JsonResult R> void PostAsync(const WebPath& path,
                                           Headers headers,
                                           Params params,
                                           OnSuccess<R> success,
                                           OnFailed failed) {
        RequestAsync(
                POST,
                path,
                headers,
                params,
                [success](const Json& content) { success(std::make_shared<R>(content)); },
                failed);
    }

    template <JsonResult R> void GetAsync(const WebPath& path,
                                          Headers headers,
                                          Params params,
                                          OnSuccess<R> success,
                                          OnFailed failed) {
        RequestAsync(
                GET,
                path,
                headers,
                params,
                [success](const Json& content) { success(std::make_shared<R>(content)); },
                failed);
    }

    template <JsonResult R>
    ResultAsync<std::shared_ptr<R>> PostAsync(const WebPath& path, Headers headers, Params params) {
        auto res = co_await RequestAsync(POST, path, headers, params);
        if (res) {
            co_return std::make_shared<R>(*res);
        } else {
            co_return unexpected(res.error());
        }
    }

    template <JsonResult R, JsonParam P> ResultAsync<std::shared_ptr<R>> PostAsync(
            const WebPath& path, Headers headers, Params params, P& content) {
        auto res = co_await RequestAsync(POST, path, headers, params, content.ToJson());
        if (res) {
            co_return std::make_shared<R>(*res);
        } else {
            co_return unexpected(res.error());
        }
    }

    template <JsonResult R, JsonParam P> ResultAsync<std::shared_ptr<R>> PostAsync(
            const WebPath& path, Headers headers, Params params, std::list<P>& content) {
        std::list<net::Json> json_array{};
        for (P& p : content) {
            json_array.push_back(p.ToJson());
        }
        net::Json json_content{json_array};
        auto res = co_await RequestAsync(POST, path, headers, params, json_content);
        if (res) {
            co_return std::make_shared<R>(*res);
        } else {
            co_return unexpected(res.error());
        }
    }

    template <JsonResult T>
    ResultAsync<std::shared_ptr<T>> GetAsync(const WebPath& path, Headers headers, Params params) {
        auto res = co_await RequestAsync(GET, path, headers, params);
        if (res) {
            co_return std::make_shared<T>(*res);
        } else {
            co_return unexpected(res.error());
        }
    }

    template <JsonResult T>
    Result<std::shared_ptr<T>> GetSync(const WebPath& path, Headers headers, Params params) {
        return SyncAwait(GetAsync<T>(path, headers, params));
    }

    template <JsonResult T>
    Result<std::shared_ptr<T>> PostSync(const WebPath& path, Headers headers, Params params) {
        return SyncAwait(PostAsync<T>(path, headers, params));
    }

    template <JsonResult R, JsonParam P> Result<std::shared_ptr<R>> PostSync(const WebPath& path,
                                                                             Headers headers,
                                                                             Params params,
                                                                             P& content) {
        return SyncAwait(PostAsync<R>(path, headers, params, content));
    }

    template <JsonResult R, JsonParam P> Result<std::shared_ptr<R>> PostSync(
            const WebPath& path, Headers headers, Params params, std::list<P>& content) {
        return SyncAwait(PostAsync<R>(path, headers, params, content));
    }

protected:
    const WebPath host;
    const bool https;
};

}  // namespace tapsdk::net
