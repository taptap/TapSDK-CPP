//
// Created by 甘尧 on 2023/9/12.
//

#include "sdk/tapsdk.h"
#include "net/httpclient.h"

namespace tapsdk::login {

struct InnerAccessToken : public AccessToken {

    explicit InnerAccessToken(const net::Json& json);

};

struct QrCodeResponse {
    std::string qrcode_url;
    std::string verification_url;
    std::string device_code;
    std::string user_code;
    s64 expires_in;
    s64 interval;

    explicit QrCodeResponse(const net::Json& json);
};

}
