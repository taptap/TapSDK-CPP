//
// Created by 甘尧 on 2023/6/30.
//

#include "base/logging.h"
#include "coro_httpclient.h"
#include "network.h"
#include "nlohmann/json.hpp"

namespace tapsdk::net {

using namespace cinatra;
using namespace nlohmann;

class JSON {
public:
    JSON(tapsdk::net::JsonStr s) {}
};

void Test() {
    constexpr auto uri = "www.baidu.com";
    CoroHttpClient client{uri, false};
    net::Params param = {{}, {}};
    client.PostAsync<JSON>(
            "",
            {},
            {},
            [](auto res) {
                LOG_ERROR("success!");
            },
            [](int status, int code, const std::string& msg) {
                LOG_ERROR("error!");
            });
}

}  // namespace tapsdk::net
