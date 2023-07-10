//
// Created by 甘尧 on 2023/7/5.
//

#pragma once

#include "httpclient.h"
#include "cinatra.hpp"

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

    ResultAsync<std::string> RequestAsync(HttpType type,
                      const WebPath& path,
                      Headers headers,
                      Params params) override;

private:
    cinatra::coro_http_client co_client{};
};

}