//
// Created by 甘尧 on 2023/7/5.
//

#pragma once

#include "cinatra.hpp"
#include "httpclient.h"

namespace tapsdk::net {

class CoroHttpClient : public TapHttpClient {
public:
    CoroHttpClient(const char* host, bool https);

    void CommonHeader(const char* key, const char* value) override;
    void CommonParam(const char* key, const char* value) override;

    void RequestAsync(HttpType type,
                      const WebPath& path,
                      Headers headers,
                      Params params,
                      OnReturn success,
                      OnFailed failed) override;

    ResultAsync<Json> RequestAsync(HttpType type,
                                   const WebPath& path,
                                   Headers headers,
                                   Params params,
                                   const Json& content = {}) override;

private:
    void InitCaCert();

    cinatra::coro_http_client co_client{};
};

}  // namespace tapsdk::net