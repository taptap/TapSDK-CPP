//
// Created by 甘尧 on 2023/7/5.
//

#include "httpclient.h"

namespace tapsdk::net {

std::string ToContent(Forms forms) {
    std::string content{};
    bool first{true};
    for (auto& [key, value] : forms) {
        if (!first) {
            content.append("&");
        } else {
            first = false;
        }
        content.append(fmt::format("{}={}", key, value));
    }
    return content;
}

ResultWrapper TapUnwrapResult(std::string_view response) {
    int code = -1;
    std::string msg{"Invalid response!"};
    Json content;
    if (response.empty()) {
        return ResultWrapper{code, msg, content};
    }
    try {
        json resp = json::parse(response);
        // old wrap
        if (resp.find("success") != resp.end()) {
            bool success = resp["success"];
            if (success) {
                code = 0;
                content = resp["data"];
            } else {
                resp = resp["data"];
                code = resp["code"];
                msg = resp["msg"];
            }
        } else {
            code = resp["code"];
            if (code == 0) {
                content = resp["data"];
            } else {
                msg = resp["msg"];
            }
        }
    } catch (...) {
        code = -1;
        msg = "Invalid response!";
    }
    return ResultWrapper{code, msg, content};
}

void TapHttpClient::SetResultUnwrap(UnwrapResult unwrap_result) {
    this->unwrap_result = unwrap_result;
}

}  // namespace tapsdk::net