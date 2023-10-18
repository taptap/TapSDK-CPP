//
// Created by 甘尧 on 2023/9/12.
//

#include "base/logging.h"
#include "core/runtime.h"
#include "login/login_model.h"
#include "login/tap_login.h"
#include "net/network.h"
#include "sdk/platform.h"

namespace tapsdk::login {

std::shared_ptr<net::TapHttpClient> http_client;
std::vector<net::Pair> common_forms;

Result<> Init(const Config& config) {
    auto current_device = platform::Device::GetCurrent();
    ASSERT_MSG(current_device, "Device = null!");
    http_client = net::CreateHttpClient("accounts.tapapis.com", true);
    net::Json info{};
    info["device_id"] = current_device->GetDeviceID();
    common_forms = {{"client_id", config.client_id},
             {"response_type", "device_code"},
             {"version", "device_code"},
             {"platform", "ue"},
             {"device_id", info.dump()}};
    return Result{};
}

static u64 TimeSecNow() {
    return Runtime::Get().Timer().OnlineTimestamp().count() / 1000;
}

static net::ResultAsync<std::shared_ptr<InnerAccessToken>> LoginAsync(const std::vector<std::string>& perm) {
    auto qrcode_result = co_await http_client->PostAsync<QrCodeResponse>("oauth2/v1/device/code", {}, {}, common_forms);
    if (!qrcode_result.has_value()) {
        co_return net::MakeError(qrcode_result.error());
    }
    // show qrcode in ui
    auto current_window = platform::Window::GetCurrent();
    if (!current_window) {
        co_return net::MakeError(-1, -1, "No window!, is current background?");
    }
    auto window_cancelable = current_window->ShowQRCode(qrcode_result.value()->qrcode_url);
    // start check state
    auto check_start_time = TimeSecNow();
    auto expires_in = qrcode_result->get()->expires_in;
    auto interval = qrcode_result->get()->interval;
    while (TimeSecNow() - check_start_time < expires_in) {
        auto token_result = co_await http_client->PostAsync<InnerAccessToken>("oauth2/v1/token", {}, {}, common_forms);
        if (token_result.has_value()) {
            co_return token_result;
        }
        co_await async_simple::coro::sleep(std::chrono::seconds(interval));
        if (window_cancelable->Canceled()) {
            co_return net::MakeError(-1, -1, "Canceled!");
        }
    }
    co_return net::MakeError(-1, -1, "Timeout!");
}

Future<AccessToken> Login(const std::vector<std::string>& perm) {
    Future<AccessToken> future{};
    LoginAsync(perm).start([future](auto result) {
                auto result_val = result.value();
                if (result_val.has_value()) {
                    future.Set(Result<AccessToken>{result_val.value()});
                } else {
                    future.Set(Result<AccessToken>{Error{result_val.error().code, result_val.error().msg}});
                }
            });
    return future;
}

}  // namespace tapsdk::login
