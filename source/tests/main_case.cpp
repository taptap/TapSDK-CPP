#include <filesystem>
#include <catch2/catch_test_macros.hpp>
#include "sdk/platform.h"
#include "sdk/tapsdk.h"

class TestDevice : public tapsdk::platform::Device {
public:
    std::string GetDeviceID() override { return "test_device_id"; }
    std::string GetCacheDir() override { return std::filesystem::current_path(); }
    std::string GetCaCertDir() override { return ""; }
};

class TestUser : public tapsdk::TDSUser {
public:
    std::string GetUserId() override { return "test_user_id"; }
    std::string GetUserName() override { return "test_device_name"; }
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
}

TEST_CASE("Test sdk-login") {
    SetupEnv();
}