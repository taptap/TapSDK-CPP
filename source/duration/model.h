//
// Created by 甘尧 on 2023/7/10.
//

#pragma once

#include "base/types.h"

namespace tapsdk::duration {
class DurPersistence;

enum EventAction {
    NULL_EVENT = 0,
    GAME_START,
    GAME_FOREGROUND,
    GAME_BACKGROUND,
    GAME_END,
    HEAT_BEAT,
    USER_LOGIN,
    USER_LOGOUT
};

struct DurEvent {
    u32 id = 0;
    u32 action;
    std::string user_id;
    std::string game_id;
    std::string session;
    u64 timestamp;
};

}  // namespace tapsdk::duration