//
// Created by 甘尧 on 2023/6/30.
//

#pragma once

#include "base/types.h"
#include <string>
#include <functional>
#include "httpclient.h"

namespace tapsdk::net {

struct DownloadResult {};

std::unique_ptr<TapHttpClient> CreateHttpClient(const char *host, bool https);
ResultAsync<DownloadResult> DownloadAsync(const char *url, const char *path);

}
