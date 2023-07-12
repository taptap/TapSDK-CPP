//
// Created by 甘尧 on 2023/6/30.
//

#include "base/logging.h"
#include "coro_httpclient.h"
#include "network.h"

namespace tapsdk::net {

std::unique_ptr<TapHttpClient> CreateHttpClient(const char *host, bool https) {
    return std::make_unique<CoroHttpClient>(host, https);
}

}  // namespace tapsdk::net
