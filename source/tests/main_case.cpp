#include <catch2/catch_test_macros.hpp>
#include "sdk/tapsdk.h"
#include "sdk/platform.h"
#include <filesystem>

class TestDevice : public tapsdk::platform::Device {
public:
    std::string GetDeviceID() override { return "test_device_id"; }
    std::string GetCacheDir() override { return std::filesystem::current_path(); }
};

class TestUser : public tapsdk::TDSUser {
public:
    std::string GetUserId() const override { return "test_user_id"; }
    std::string GetUserName() const override { return "test_device_name"; }
};

static void SetupEnv() {
    tapsdk::platform::Device::SetCurrent(std::make_shared<TestDevice>());
    tapsdk::TDSUser::SetCurrent(std::make_shared<TestUser>());
}

TEST_CASE("Test sdk-init") {
    SetupEnv();
    REQUIRE(tapsdk::Init());
}

TEST_CASE("Test sdk-login") {
    SetupEnv();
    class : public tapsdk::LoginCallback {
        void OnSuccess(const std::shared_ptr<tapsdk::TDSUser>& user) override {

        }
        void OnFailed(int err_code, const char* msg) override {

        }
    } callback{};
    tapsdk::Login("xxx", "xxxx", &callback);
}