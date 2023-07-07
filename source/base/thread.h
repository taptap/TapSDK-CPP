//
// Created by 甘尧 on 2023/7/7.
//

#pragma once

#include "types.h"

namespace tapsdk {

enum class ThreadPriority : u32 {
    Low = 0,
    Normal = 1,
    High = 2,
    VeryHigh = 3,
    Critical = 4,
};

void SetCurrentThreadPriority(ThreadPriority new_priority);

void SetCurrentThreadName(const char* name);

}  // namespace tapsdk
