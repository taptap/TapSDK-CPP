#include <filesystem>
#include <iostream>
#include <thread>
#include <catch2/catch_test_macros.hpp>
#include "externals/libqrencode/qrencode.h"
#include "sdk/platform.h"
#include "sdk/tapsdk.h"

class TestDevice : public tapsdk::platform::Device {
public:
    std::string GetDeviceID() override { return "test_device_id"; }
    std::string GetCacheDir() override { return std::filesystem::current_path().string(); }
    std::string GetCaCertDir() override { return ""; }
};

class TestUser : public tapsdk::TDSUser {
public:
    bool ContainTapInfo() override { return false; }
    std::string GetUserId() override { return "{\n"
                                              "    \"tds_id\":\"xxxx\",\n"
                                              "    \"open_id\":\"xxxx\"\n"
                                              "}"; }
    std::string GetUserName() override { return "test_device_name"; }
};

class TestGame : public tapsdk::Game {
public:
    std::string GetGameID() override { return "test_game_id"; }
    std::string GetPackageName() override { return "test_game_pkg"; }
};

class TestWindow : public tapsdk::platform::Window {
public:

    void PrintQR(QRcode *qrcode) {
        unsigned char *p = qrcode->data;
        for(int y=0; y<qrcode->width; y++) {
            for(int x=0; x<qrcode->width; x++) {
                std::cout << ((*p & 1) ? "##" : "  ");
                p++;
            }
            std::cout << "\n";
        }
    }

    std::shared_ptr<tapsdk::platform::Cancelable> ShowQRCode(const std::string& qr_code) override {
        QRcode *qrcode;
        qrcode = QRcode_encodeString(qr_code.c_str(), 2, QR_ECLEVEL_L, QR_MODE_8, 1);
        PrintQR(qrcode);
        QRcode_free(qrcode);
        return std::make_shared<tapsdk::platform::Cancelable>();
    }
};

static void SetupEnv() {
    tapsdk::platform::Device::SetCurrent(std::make_shared<TestDevice>());
    tapsdk::platform::Window::SetCurrent(std::make_shared<TestWindow>());
}

TEST_CASE("Test sdk-login") {
    SetupEnv();
    tapsdk::Config config {
            .enable_tap_login = true,
            .enable_duration_statistics = false,
            .client_id = "0RiAlMny7jiz086FaU"
    };
    tapsdk::Init(config);
    tapsdk::TDSUser::SetCurrent(std::make_shared<TestUser>());
    tapsdk::Game::SetCurrent(std::make_shared<TestGame>());
    auto login_result = *tapsdk::Login({});
    while (true) {
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }
}