//
// Created by 甘尧 on 2023/9/14.
//

#pragma once

#include "base/types.h"
#include "sdk/platform.h"
#include "sdk/tapsdk.h"

namespace tapsdk::tracker {

void Init(const Config &config);

std::shared_ptr<TrackMessage> CreateTracker(const std::shared_ptr<TrackerConfig> &config);
void FlushTracker(const std::shared_ptr<TrackMessage> &tracker);

}
