//
// Created by 甘尧 on 2023/9/25.
//

#pragma once

#include "net/network.h"
#include "sdk/tapsdk.h"
#include "base/file.h"

namespace tapsdk::tracker {

class TrackCache {
public:
    explicit TrackCache(std::string topic, std::string path);

    void Init();

private:
    std::string topic;
    std::string path;
    std::mutex lock;
    std::unique_ptr<File> index_file;
    std::unique_ptr<File> content_file;
};

}
