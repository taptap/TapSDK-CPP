#include <catch2/catch_test_macros.hpp>
#include "sdk/tapsdk.h"

TEST_CASE("Test sdk-init") {
    REQUIRE(tapsdk::Init());
}

TEST_CASE("Test sdk-login") {
    class : public tapsdk::LoginCallback {
        void OnSuccess(const std::shared_ptr<tapsdk::TDSUser>& user) override {

        }
        void OnFailed(int err_code, const char* msg) override {

        }
    } callback{};
    tapsdk::Login("xxx", "xxxx", &callback);
}