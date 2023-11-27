#include <filesystem>
#include <iostream>
#include <thread>
#include <catch2/catch_test_macros.hpp>
#include "externals/libqrencode/qrencode.h"
#include "sdk/platform.h"
#include "sdk/tapsdk.h"
#include "sdk/tapsdk_api.h"

class TestDevice : public tapsdk::platform::Device {
public:
    std::string GetDeviceID() override { return "test_device_id"; }
    std::string GetCacheDir() override { return std::filesystem::current_path().string(); }
    std::string GetCaCertDir() override { return ""; }
    std::shared_ptr<tapsdk::platform::DeviceInfo> GetDeviceInfo() override { return std::make_shared<tapsdk::platform::DeviceInfo>(); }
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
    std::string GetGameID() override { return "0RiAlMny7jiz086FaU"; }
    std::string GetPackageName() override { return "test_game_pkg"; }
    std::string GetVersion() override { return ""; }
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

TEST_CASE("Test sdk cpp interface") {
    SetupEnv();
    tapsdk::Config config {
            .enable_tap_login = true,
            .enable_duration_statistics = true,
            .enable_tap_tracker = true,
            .process_name = "main_process"
    };

    auto tracker_config = std::make_shared<tapsdk::TrackerConfig>();
    tracker_config->topic = "tds_topic";
    tracker_config->endpoint = "openlog.xdrnd.com";
    tracker_config->access_keyid = "${You ID}";
    tracker_config->access_key_secret = "${You Key}";
    tracker_config->project = "tds";
    tracker_config->log_store = "tapsdk_us";

    tapsdk::Init(config);
    tapsdk::TDSUser::SetCurrent(std::make_shared<TestUser>());
    tapsdk::TDSUser::SetCurrent({});
    tapsdk::Game::SetCurrent(std::make_shared<TestGame>());
//    auto login_result = *tapsdk::Login({});
    auto ta = tapsdk::CreateTracker(tracker_config);
    ta->AddContent("page_id", "page_game");
    ta->AddContent("login_action", "taptap_authorization_start");
    ta->AddContent("page_name", "游戏");
    ta->AddContent("time", "1695086167");

    tapsdk::FlushTracker(ta);

    while (1) {
        std::this_thread::sleep_for(std::chrono::minutes(1));
    }
}

TEST_CASE("Test sdk c interface") {
    auto path = std::filesystem::current_path();
    tapsdk_device device {
            .device_id = "test_device_id",
            .cache_dir = const_cast<char*>(path.c_str()),
    };
    tapsdk_config config {
            .enable_duration_statistics = true,
            .enable_tap_tracker = true,
            .region = TDS_REGION_GLOBAL,
            .process_name = "main",
            .device = &device
    };
    assert(tapsdk_init(&config) == TAPSDK_SUCCESS);

    tapsdk_game game {
            .client_id = "0RiAlMny7jiz086FaU",
            .identify = "com.test.game"
    };
    assert(tapsdk_game_set(&game) == TAPSDK_SUCCESS);

    tapsdk_user user {
            .user_id = "{\n"
                    "    \"tds_id\":\"xxxx\",\n"
                    "    \"open_id\":\"xxxx\"\n"
                    "}",
            .contain_tap_info = false
    };
    assert(tapsdk_user_set(&user) == TAPSDK_SUCCESS);

    tapsdk_tracker_config tracker_config {};
    tracker_config.topic = "tds_topic";
    tracker_config.endpoint = "openlog.xdrnd.com";
    tracker_config.access_keyid = "${You ID}";
    tracker_config.access_key_secret = "${You Key}";
    tracker_config.project = "tds";
    tracker_config.log_store = "tapsdk_us";

    tapsdk_tracker_message *message;
    assert(tapsdk_tracker_create(&tracker_config, &message) == TAPSDK_SUCCESS);

    assert(tapsdk_tracker_msg_add_param(message, "test_key", "test_value") == TAPSDK_SUCCESS);
    assert(tapsdk_tracker_msg_add_content(message, "test_key", "test_value") == TAPSDK_SUCCESS);

    assert(tapsdk_tracker_flush(message) == TAPSDK_SUCCESS);

    while (1) {
        std::this_thread::sleep_for(std::chrono::minutes(1));
    }
}