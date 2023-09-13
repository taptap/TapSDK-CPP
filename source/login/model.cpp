//
// Created by 甘尧 on 2023/9/12.
//

#include "model.h"

namespace tapsdk::login {

InnerAccessToken::InnerAccessToken(const net::Json& json) {
    kid = "";
}

QrCodeResponse::QrCodeResponse(const net::Json& json) {
    qrcode_url = json["qrcode_url"];
    verification_url = json["verification_url"];
    device_code = json["device_code"];
    user_code = json["user_code"];
    expires_in = json["expires_in"];
    interval = json["interval"];
}

}
