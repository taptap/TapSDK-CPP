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

    void CommonHeader(std::string_view key, std::string_view value) override;
    void CommonParam(std::string_view key, std::string_view value) override;

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
                                   Content content = {},
                                   ContentType content_type = {}) override;

private:
    std::shared_ptr<cinatra::coro_http_client> AcquireClient();
    void RecycleClient(const std::shared_ptr<cinatra::coro_http_client>& client);

    std::unordered_map<std::string, std::string> common_headers;
    std::unordered_map<std::string, std::string> common_params;
    std::mutex client_lock;
    std::queue<std::shared_ptr<cinatra::coro_http_client>> client_pool;
};

}  // namespace tapsdk::net