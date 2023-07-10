//
// Created by 甘尧 on 2023/7/7.
//

#pragma once

#include "base/timer.h"

namespace tapsdk {

class Runtime {
public:
    static Runtime& Get();

    void Init();

    CoreTimer& Timer() { return timer; }

private:
    CoreTimer timer;
};

}  // namespace tapsdk
