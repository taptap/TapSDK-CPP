//
// Created by 甘尧 on 2023/9/25.
//

#include "cache.h"

#include <utility>

namespace tapsdk::tracker {

struct CacheEntry {
    u32 offset;
    u32 length;
    u64 time;
};

struct CacheIndex {
    std::array<u8, 4> magic;
    u16 ver_code;
    u32 entry_size;
};

struct CacheEntries {
    std::array<u8, 4> magic;
    u16 ver_code;
    u32 size;
};

constexpr std::array<u8, 4> cache_magic = {'t', 'd', 's', 'c'};
constexpr auto cache_version_code = 1;
constexpr auto max_track_counts = 10000;
constexpr auto index_file_size = sizeof(CacheIndex) + max_track_counts * sizeof(CacheEntry);
constexpr auto max_cache_size = 1024 * 1024 * 8;

bool CacheToDisk(const std::string &topic, const std::list<std::shared_ptr<TrackMessage>> &msgs) {
    return true;
}

TrackCache::TrackCache(std::string topic, std::string path)
        : topic(std::move(topic)), path(std::move(path)) {}

void TrackCache::Init() {

}

}