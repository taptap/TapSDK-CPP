#include <filesystem>
#include <catch2/catch_test_macros.hpp>
#include "sdk/platform.h"
#include "sdk/tapsdk.h"

class TestDevice : public tapsdk::platform::Device {
public:
    std::string GetDeviceID() override { return "test_device_id"; }
    std::string GetCacheDir() override { return std::filesystem::current_path(); }
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
    tapsdk::Init();
    tapsdk::TDSUser::SetCurrent(std::make_shared<TestUser>());
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