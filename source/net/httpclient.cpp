//
// Created by 甘尧 on 2023/7/5.
//

#include "httpclient.h"
#include "nlohmann/json.hpp"

using namespace nlohmann;

namespace tapsdk::net {

TapResult::TapResult(const char* response) {
    if (!response) {
        code = -1;
        msg = "No response!";
        return;
    }
    try {
        json resp = json::parse(response);
        code = resp["code"];
        msg = resp["msg"];
        content = resp["content"];
    } catch (...) {
        code = -1;
        msg = "Invalid response!";
    }
}

int TapResult::GetCode() const { return code; }
const std::string& TapResult::GetMsg() const { return msg; }
const std::string& TapResult::GetContent() const { return content; }

}  // namespace tapsdk::net