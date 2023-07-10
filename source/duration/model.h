//
// Created by 甘尧 on 2023/7/10.
//

#pragma once

#include "base/types.h"

namespace tapsdk::duration {
class DurPersistence;

struct DurEvent {
    std::string session;
    u64 timestamp;
};

}  // namespace tapsdk::duration