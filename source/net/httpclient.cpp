//
// Created by 甘尧 on 2023/7/5.
//

#include "httpclient.h"

namespace tapsdk::net {

ResultWrap::ResultWrap(std::string_view response) {
    if (response.empty()) {
        code = -1;
        msg = "No response!";
        return;
    }
    try {
        json resp = json::parse(response);
        code = resp["code"];
        if (code == 0) {
            content = resp["data"];
        } else {
            msg = resp["msg"];
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