//
// Created by 甘尧 on 2023/9/25.
//

#pragma once

#include "base/file.h"
#include "net/network.h"
#include "sdk/tapsdk.h"
#include "tracker/tracker_impl.h"

namespace tapsdk::tracker {

#pragma pack(push, 4)
struct CacheHeader {
    std::array<u8, 4> magic;
    u32 ver_code;
    u64 config_hash;
    u32 config_size;
    u32 record_count;
    u32 record_size;
};

struct RecordHeader {
    std::array<u8, 4> magic;
    u64 hash;
    u32 length;
    u32 time;
};
#pragma pack(pop)

class DiskCache : DeleteCopyAndMove {
public:
    constexpr static auto config_offset = sizeof(CacheHeader);

    explicit DiskCache(u64 hash, std::string path, const std::shared_ptr<TrackerConfig>& config = {});

    ~DiskCache() override;

    void Init();

    std::shared_ptr<TrackerConfig> GetConfig();

    bool Push(TrackMessageImpl &cache);

    std::list<TrackMessageImpl> Load();

    u32 GetCount() const;

    void Clear();

    bool Destroy();

private:
    bool IsMapped();

    void LoadConfig();

    void SaveConfig();

    void Reset();

    u64 hash;
    std::string path;
    std::shared_ptr<TrackerConfig> config;

    std::mutex lock;
    std::unique_ptr<File> file;
    CacheHeader header_buf{};
    CacheHeader* cache_header = &header_buf;
};

}  // namespace tapsdk::tracker
