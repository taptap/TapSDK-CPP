//
// Created by 甘尧 on 2023/7/7.
//

#include "base/spin_lock.h"
#include <thread>

#if _MSC_VER
#include <intrin.h>
#if _M_AMD64
#define __x86_64__ 1
#endif
#if _M_ARM64
#define __aarch64__ 1
#endif
#else
#if __x86_64__
#include <xmmintrin.h>
#endif
#endif

namespace {

void ThreadPause() { std::this_thread::yield(); }

}  // Anonymous namespace

namespace tapsdk {

void SpinLock::lock() {
    while (lck.test_and_set(std::memory_order_acquire)) {
        ThreadPause();
    }
}

void SpinLock::unlock() { lck.clear(std::memory_order_release); }

bool SpinLock::try_lock() {
    if (lck.test_and_set(std::memory_order_acquire)) {
        return false;
    }
    return true;
}

}  // namespace tapsdk
