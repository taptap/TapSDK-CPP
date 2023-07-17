//
// Created by 甘尧 on 2023/7/5.
//

#include "httpclient.h"

namespace tapsdk::net {

ResultWrap::ResultWrap(const char* response) {
    if (!response) {
        code = -1;
        msg = "No response!";
        return;
    }
    try {
        json resp = json::parse(response);
        content = resp["content"];
        if (resp["success"].get<bool>()) {
            code = 0;
        } else {
            code = content["code"];
            msg = content["msg"];
        }
    } catch (...) {
        code = -1;
        msg = "Invalid response!";
    }
}

int ResultWrap::GetCode() const { return code; }
const std::string& ResultWrap::GetMsg() const { return msg; }
const Json& ResultWrap::GetContent() const { return content; }

}  // namespace tapsdk::net