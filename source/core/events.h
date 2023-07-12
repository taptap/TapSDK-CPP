//
// Created by 甘尧 on 2023/7/10.
//

#pragma once

#include "sdk/tapsdk.h"

namespace tapsdk::events {

struct User {
    std::shared_ptr<TDSUser> user;
};

struct Foreground {};

struct Background {};

}
