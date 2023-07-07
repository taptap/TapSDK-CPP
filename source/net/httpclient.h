//
// Created by 甘尧 on 2023/7/5.
//

#pragma once

#include <memory>
#include <string>
#ifdef ANDROID
#include "externals/compat/concepts.h"
#else
#include <concepts>
#endif
#include "base/types.h"

namespace tapsdk::net {

constexpr auto http_timeout_ms = 10 * 1000; // ms

enum HttpType { GET, POST };

using Pair = std::pair<std::string, std::string>;
using OnReturn = const std::function<void(const std::string& content)>&;
template <class T> using OnSuccess = const std::function<void(const std::shared_ptr<T>& content)>&;
using OnFailed = const std::function<void(int status, int code, const std::string& msg)>&;
using Params = std::initializer_list<Pair>;
using Headers = std::initializer_list<Pair>;
using JsonStr = std::string_view;

class TapResult {
public:
    explicit TapResult(const char* response);

    [[nodiscard]] int GetCode() const;
    [[nodiscard]] const std::string& GetMsg() const;
    [[nodiscard]] const std::string& GetContent() const;

private:
    int code;
    std::string msg;
    std::string content;
};

template <typename T>
concept JsonResult = requires(JsonStr str) {
    { T(str) };
};

template <typename T>
concept JsonParam = requires(T t) {
    { t.ToJson() } -> std::convertible_to<JsonStr>;
};

class TapHttpClient {
public:
    TapHttpClient(const char* host, bool https) : host{host}, https{https} {};
    virtual void CommonHeader(const char* key, const char* value) = 0;
    virtual void CommonParam(const char* key, const char* value) = 0;

    virtual void RequestAsync(HttpType type,
                              const WebPath& path,
                              Headers headers,
                              Params params,
                              OnReturn success,
                              OnFailed failed) = 0;

    template <JsonResult T> void PostAsync(const WebPath& path,
                                           Headers headers,
                                           Params params,
                                           OnSuccess<T> success,
                                           OnFailed failed) {
        RequestAsync(
                POST,
                path,
                headers,
                params,
                [success](const std::string& content) { success(std::make_shared<T>(content)); },
                failed);
    }

    template <JsonResult T> void GetAsync(const WebPath& path,
                                          Headers headers,
                                          Params params,
                                          OnSuccess<T> success,
                                          OnFailed failed) {
        RequestAsync(
                GET,
                path,
                headers,
                params,
                [success](const std::string& content) { success(std::make_shared<T>(content)); },
                failed);
    }

protected:
    const WebPath host;
    const bool https;
};

}  // namespace tapsdk::net
