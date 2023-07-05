//
// Created by 甘尧 on 2023/6/30.
//

#include "network.h"
#include "cinatra.hpp"
#include "base/logging.h"
#include "nlohmann/json.hpp"

namespace tapsdk::net {

using namespace cinatra;
using namespace nlohmann;

void Test() {
    constexpr auto uri = "http://www.baidu.com";
    coro_http_client client{};
    auto result = client.get(uri);
    ASSERT(!result.net_err);
    LOG_DEBUG("request body {}", result.resp_body);

    result = client.post(uri, "hello", req_content_type::json);
    LOG_DEBUG("response body {}", result.resp_body);
}

}
