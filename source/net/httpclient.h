//
// Created by 甘尧 on 2023/7/5.
//

#pragma once

#include <list>
#include <memory>
#include <span>
#include <string>

#ifdef ANDROID

#include "externals/compat/concepts.h"

#else
#include <concepts>
#endif

#include "async_simple/coro/Lazy.h"
#include "async_simple/coro/Sleep.h"
#include "async_simple/coro/SyncAwait.h"
#include "base/expected.h"
#include "base/types.h"
#include "nlohmann/json.hpp"

namespace tapsdk::net {

using namespace nlohmann;
constexpr auto http_timeout_ms = 10 * 1000;  // ms

enum HttpType { GET, POST };
enum ContentType { NONE, JSON, FORM };

static_assert(sizeof(char) == sizeof(u8));
using Content = std::span<char>;
using Json = nlohmann::json;
using Pair = std::pair<std::string, std::string>;
using OnReturn = const std::function<void(const Json& content)>&;
template <class T> using OnSuccess = const std::function<void(const std::shared_ptr<T>& content)>&;
using OnFailed = const std::function<void(int status, int code, const std::string& msg)>&;
using Params = std::span<Pair>;
using Headers = std::span<Pair>;
using Forms = std::span<Pair>;

struct ResultWrapper {
    int code;
    std::string msg;
    Json content{};
};

using UnwrapResult = ResultWrapper (*)(std::string_view response);

ResultWrapper TapUnwrapResult(std::string_view response);

std::string ToContent(Forms forms);

struct Error {
    int status;
    int code;
    std::string msg;
};

template <typename T> using Result = expected<T, Error>;
template <typename T> using ResultAsync = async_simple::coro::Lazy<Result<T>>;
template <typename T> using ResultCallback = async_simple::Try<Result<T>>;

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

inline unexpected<Error> MakeError(int status, int code, const std::string& msg) {
    return unexpected(Error{status, code, msg});
}

inline unexpected<Error> MakeError(const Error& error) { return unexpected(error); }

template <JsonResult T> inline Result<std::shared_ptr<T>> MakeResult(Result<Json>& res) {
    if (res) {
        try {
            return std::make_shared<T>(*res);
        } catch (std::exception& e) {
            return MakeError(200, -1, e.what());
        }
    } else {
        return unexpected(res.error());
    }
}

class TapHttpClient {
public:
    TapHttpClient(const char* host, bool https) : host{host}, https{https}, unwrap_result{&TapUnwrapResult} {};

    virtual ~TapHttpClient() = default;

    virtual void CommonHeader(std::string_view key, std::string_view value) = 0;

    virtual void CommonParam(std::string_view key, std::string_view value) = 0;

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
                                           Content content = {},
                                           ContentType content_type = {}) = 0;

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
    ResultAsync<std::shared_ptr<R>> PostAsync(const WebPath& path, Headers headers, Params params, Content content = {}, ContentType type = ContentType::NONE) {
        auto res = co_await RequestAsync(POST, path, headers, params, content, type);
        co_return MakeResult<R>(res);
    }

    template <JsonResult R> ResultAsync<std::shared_ptr<R>> PostAsync(const WebPath& path,
                                                                      Headers headers,
                                                                      Params params,
                                                                      Forms forms) {
        auto form_str = ToContent(forms);
        auto res = co_await RequestAsync(POST, path, headers, params, form_str, FORM);
        co_return MakeResult<R>(res);
    }

    template <JsonResult R, JsonParam P> ResultAsync<std::shared_ptr<R>> PostAsync(
            const WebPath& path, Headers headers, Params params, P& content) {
        Json json_content;
        try {
            json_content = content.ToJson();
        } catch (std::exception& e) {
            co_return MakeError(-1, -1, e.what());
        }
        auto json_str = json_content.dump();
        auto res = co_await RequestAsync(POST, path, headers, params, json_str, JSON);
        co_return MakeResult<R>(res);
    }

    template <JsonResult R, JsonParam P> ResultAsync<std::shared_ptr<R>> PostAsync(
            const WebPath& path, Headers headers, Params params, std::list<P>& content) {
        net::Json json_content{};
        for (P& p : content) {
            try {
                json_content.push_back(p.ToJson());
            } catch (std::exception& e) {
                co_return MakeError(-1, -1, e.what());
            }
        }
        auto json_str = json_content.dump();
        auto res = co_await RequestAsync(POST, path, headers, params, json_str, JSON);
        co_return MakeResult<R>(res);
    }

    template <JsonResult T>
    ResultAsync<std::shared_ptr<T>> GetAsync(const WebPath& path, Headers headers, Params params) {
        auto res = co_await RequestAsync(GET, path, headers, params);
        co_return MakeResult<T>(res);
    }

    template <JsonResult T> ResultAsync<std::shared_ptr<T>> GetAsync(const WebPath& path,
                                                                     Headers headers,
                                                                     Params params,
                                                                     Forms forms) {
        auto form_str = ToContent(forms);
        auto res = co_await RequestAsync(GET, path, headers, params, form_str, FORM);
        co_return MakeResult<T>(res);
    }

    template <JsonResult T>
    Result<std::shared_ptr<T>> GetSync(const WebPath& path, Headers headers, Params params) {
        return SyncAwait(GetAsync<T>(path, headers, params));
    }

    template <JsonResult T> Result<std::shared_ptr<T>> GetSync(const WebPath& path,
                                                               Headers headers,
                                                               Params params,
                                                               Forms forms) {
        return SyncAwait(GetAsync<T>(path, headers, params, forms));
    }

    template <JsonResult T>
    Result<std::shared_ptr<T>> PostSync(const WebPath& path, Headers headers, Params params) {
        return SyncAwait(PostAsync<T>(path, headers, params));
    }

    template <JsonResult T> Result<std::shared_ptr<T>> PostSync(const WebPath& path,
                                                                Headers headers,
                                                                Params params,
                                                                Forms forms) {
        return SyncAwait(PostAsync<T>(path, headers, params, forms));
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

    void SetResultUnwrap(UnwrapResult unwrap_result);

protected:
    const WebPath host;
    const bool https;
    UnwrapResult unwrap_result;
};

}  // namespace tapsdk::net
