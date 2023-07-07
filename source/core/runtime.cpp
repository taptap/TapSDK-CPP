//
// Created by 甘尧 on 2023/7/7.
//

#include "runtime.h"

namespace tapsdk {

Runtime& Runtime::Get() {
    static Runtime runtime{};
    return runtime;
}

void Runtime::Init() {
    timer.Start();
}

}  // namespace tapsdk
