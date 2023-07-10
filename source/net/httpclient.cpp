//
// Created by 甘尧 on 2023/7/5.
//

#include "httpclient.h"
#include "nlohmann/json.hpp"

using namespace nlohmann;

namespace tapsdk::net {

ResultWrap::ResultWrap(const char* response) {
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

int ResultWrap::GetCode() const { return code; }
const std::string& ResultWrap::GetMsg() const { return msg; }
const std::string& ResultWrap::GetContent() const { return content; }

}  // namespace tapsdk::net