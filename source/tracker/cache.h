//
// Created by 甘尧 on 2023/9/25.
//

#pragma once

#include "base/file.h"
#include "net/network.h"
#include "sdk/tapsdk.h"

namespace tapsdk::tracker {

#pragma pack(push, 4)
struct CacheEntry {
    u32 offset;
    u32 length;
    u32 time;
};

struct CacheIndex {
    std::array<u8, 4> magic;
    u32 ver_code;
    u32 entry_size;
};

struct CacheEntries {
    std::array<u8, 4> magic;
    u32 ver_code;
    u32 size;
};
#pragma pack(pop)

class TrackCache : DeleteCopyAndMove {
public:
    explicit TrackCache(std::string topic, std::string path);

    void Init();

    bool Push(u64 time, std::span<u8> content);

    virtual ~TrackCache();

private:
    std::string topic;
    std::string path;
    std::mutex lock;
    std::unique_ptr<File> index_file;
    std::unique_ptr<File> content_file;

    CacheIndex* index_header{};
    CacheEntries* entries_header{};
    std::span<CacheEntry> entries{};

    CacheIndex idx_header_buf{};
    CacheEntries cnt_header_buf{};
};

}  // namespace tapsdk::tracker
