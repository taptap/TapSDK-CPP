#include <filesystem>
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
    std::string GetUserId() override { return "test_user_id"; }
    std::string GetUserName() override { return "test_device_name"; }
};

class TestGame : public tapsdk::Game {
public:
    std::string GetGameID() override { return "test_game_id"; }
    std::string GetPackageName() override { return "test_game_pkg"; }
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
}

TEST_CASE("Test sdk-login") {
    SetupEnv();
}