//
// Created by 甘尧 on 2023/7/5.
//

#pragma once

#include <unordered_map>
#include <queue>
#include "cinatra.hpp"
#include "httpclient.h"

namespace tapsdk::net {

class CoroHttpClient : public TapHttpClient {
public:
    CoroHttpClient(const char* host, bool https);

    void CommonHeader(const char* key, const char* value) override;
    void CommonParam(const char* key, const char* value) override;

    void RequestAsync(HttpType type,
                      WebPath path,
                      Headers headers,
                      Params params,
                      OnReturn success,
                      OnFailed failed) override;

    ResultAsync<Json> RequestAsync(HttpType type,
                                   WebPath path,
                                   Headers headers,
                                   Params params,
                                   const Json& content = {}) override;

private:

    std::shared_ptr<cinatra::coro_http_client> AcquireClient();
    void RecycleClient(const std::shared_ptr<cinatra::coro_http_client> &client);

    std::unordered_map<std::string, std::string> headers;
    std::unordered_map<std::string, std::string> params;
    std::mutex client_lock;
    std::queue<std::shared_ptr<cinatra::coro_http_client>> client_pool;
};

}  // namespace tapsdk::net