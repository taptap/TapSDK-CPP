#include <filesystem>
#include <thread>
#include <catch2/catch_test_macros.hpp>
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
    void ShowQRCode(const std::string& path) override {}
};

static void SetupEnv() {
    tapsdk::platform::Device::SetCurrent(std::make_shared<TestDevice>());
}

TEST_CASE("Test sdk-init") {
    SetupEnv();
    tapsdk::Config config {
            .enable_duration_statistics = true,
    };
    tapsdk::Init(config);
    tapsdk::TDSUser::SetCurrent(std::make_shared<TestUser>());
    tapsdk::Game::SetCurrent(std::make_shared<TestGame>());
    while (true) {
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }
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