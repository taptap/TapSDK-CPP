//
// Created by 甘尧 on 2023/9/25.
//

#include <utility>
#include "base/alignment.h"
#include "base/cityhash.h"
#include "base/logging.h"
#include "cache.h"
#include "track_config_cache.pb.h"
#include "tracker_impl.h"

#ifdef __APPLE__
#include <libkern/OSCacheControl.h>
#endif

namespace tapsdk::tracker {

constexpr std::array<u8, 4> cache_magic = {'t', 'd', 's', 'c'};
constexpr std::array<u8, 4> record_magic = {'t', 'r', 'c', 'd'};
constexpr auto cache_version_code = 1;
constexpr auto max_cache_size = 1024 * 1024 * 1;
constexpr auto cache_capability = 1024 * 64;

static void ClearDCache(void* start, size_t size) {
#ifdef __APPLE__
    sys_dcache_flush(reinterpret_cast<char*>(start), size);
#endif
}

DiskCache::DiskCache(u64 hash, std::string path, const std::shared_ptr<TrackerConfig>& config)
        : hash(hash), path(std::move(path)), config(config) {
    file = std::make_unique<File>(this->path);
}

void DiskCache::Init() {
    ASSERT_MSG(
            file->Open(FileAccessMode::ReadWrite), "Cache file {} open failed!", file->GetPath());

    auto should_init{false};

    if (file->Size() < cache_capability) {
        ASSERT(file->Resize(cache_capability));
        should_init = true;
    }

    if (auto file_memory = file->Map(0, file->Size()); file_memory) {
        cache_header = reinterpret_cast<CacheHeader*>(file_memory);
    } else {
        *cache_header = file->Read<CacheHeader>();
    }

    if (should_init || !CheckValid()) {
        Reset();
    } else if (!config) {
        LoadConfig();
    }
}

std::shared_ptr<TrackerConfig> DiskCache::GetConfig() { return config; }

u32 DiskCache::GetCount() const { return cache_header->record_count; }

bool DiskCache::Push(TrackMessageImpl& cache) {
    auto cache_size = cache.GetSerializeSize();
    std::lock_guard guard(lock);
    auto next_size = sizeof(CacheHeader) + cache_header->config_size + cache_header->record_size +
                     sizeof(RecordHeader) + cache_size;
    if (next_size > max_cache_size) {
        return false;
    }
    if (next_size > file->Size()) {
        auto old_size = file->Size();
        auto new_size = file->Size() + cache_capability;
        if (!file->Resize(new_size)) {
            return false;
        }
        if (IsMapped()) {
            if (auto new_header = reinterpret_cast<CacheHeader*>(file->Map(0, new_size));
                new_header) {
                if (!Unmap(cache_header, old_size)) {
                    return false;
                }
                cache_header = new_header;
            } else {
                return false;
            }
        }
    }
    auto write_offset = sizeof(CacheHeader) + cache_header->config_size + cache_header->record_size;
    RecordHeader header{.magic = record_magic,
                        .hash = 0,
                        .length = static_cast<u32>(cache_size),
                        .time = cache.GetCreateTime()};
    if (IsMapped()) {
        auto memory = reinterpret_cast<u8*>(cache_header) + write_offset;
        std::span<u8> content{memory + sizeof(header), cache_size};
        if (!cache.SerializeToBuffer(content)) {
            return false;
        }
        header.hash = CityHash64(reinterpret_cast<const char*>(content.data()), cache_size);
        std::memcpy(memory, &header, sizeof(header));
        cache_header->record_count++;
        cache_header->record_size += cache_size + sizeof(header);
        return file->Commit();
    } else {
        std::vector<u8> content(cache_size);
        if (!cache.SerializeToBuffer(content)) {
            return false;
        }
        header.hash = CityHash64(reinterpret_cast<const char*>(content.data()), cache_size);
        if (!file->Write(header, write_offset)) {
            return false;
        }
        if (!file->Write(content.data(), write_offset + sizeof(header), cache_size)) {
            return false;
        }
        cache_header->record_count++;
        cache_header->record_size += cache_size + sizeof(header);
        if (!file->Write(cache_header)) {
            return false;
        }
        return file->Flush();
    }
}

std::list<TrackMessageImpl> DiskCache::Load() {
    std::lock_guard guard(lock);
    std::list<TrackMessageImpl> result{};
    if (sizeof(CacheHeader) + cache_header->config_size + cache_header->record_size >
        file->Size()) {
        return result;
    }
    std::vector<u8> buffer;
    u8* memory_start;
    if (IsMapped()) {
        memory_start = reinterpret_cast<u8*>(cache_header) + sizeof(CacheHeader) +
                       cache_header->config_size;
    } else {
        buffer.resize(cache_header->record_size);
        memory_start = buffer.data();
        file->Read(memory_start,
                   sizeof(CacheHeader) + cache_header->config_size,
                   cache_header->record_size);
    }
    u32 read_size{0};
    u32 cur_offset{0};
    for (u32 i = 0; i < cache_header->record_count && read_size < cache_header->record_size; ++i) {
        auto memory = memory_start + cur_offset;
        auto record_header = reinterpret_cast<RecordHeader*>(memory);
        auto record_memory = reinterpret_cast<char*>(memory + sizeof(RecordHeader));
        if (cur_offset + sizeof(RecordHeader) + record_header->length > cache_header->record_size) {
            return result;
        }
        read_size += sizeof(RecordHeader) + record_header->length;
        cur_offset += sizeof(RecordHeader) + record_header->length;
        if (record_header->magic != record_magic) {
            continue;
        }
        if (record_header->hash != CityHash64(record_memory, record_header->length)) {
            continue;
        }
        std::span<u8> data{reinterpret_cast<u8*>(record_memory), record_header->length};
        auto& ref = result.emplace_back(config);
        if (!ref.Deserialize(data)) {
            result.erase(std::prev(result.end()));
        }
    }
    return result;
}

void DiskCache::Clear() { Reset(); }

bool DiskCache::Destroy() { return file->Close() && File::Delete(path); }

bool DiskCache::CheckValid() {
    if (cache_header->magic != cache_magic || cache_header->config_hash != hash) {
        return false;
    }
    if (cache_header->ver_code != cache_version_code) {
        return false;
    }
    if (sizeof(CacheHeader) + cache_header->config_size + cache_header->record_size >
        file->Size()) {
        return false;
    }
    return true;
}

void DiskCache::Reset() {
    cache_header->magic = cache_magic;
    cache_header->ver_code = cache_version_code;
    cache_header->record_count = 0;
    cache_header->record_size = 0;
    if (IsMapped()) {
        ASSERT(file->Commit());
    } else {
        ASSERT(file->Write(*config));
        ASSERT(file->Flush());
    }
    if (file->Size() > cache_capability) {
        if (IsMapped()) {
            ASSERT(Unmap(cache_header, file->Size()));
        }
        ASSERT(file->Resize(cache_capability));
        if (IsMapped()) {
            cache_header = reinterpret_cast<CacheHeader*>(file->Map(0, cache_capability));
            ASSERT(cache_header);
        }
    }
    SaveConfig();
}

bool DiskCache::IsMapped() { return cache_header != &header_buf; }

void DiskCache::LoadConfig() {
    TrackDiskCacheConfig disk_cache_config{};
    ASSERT(config_offset + cache_header->config_size <= file->Size());
    if (IsMapped()) {
        auto memory = reinterpret_cast<u8*>(cache_header) + config_offset;
        ASSERT(disk_cache_config.ParseFromArray(memory, cache_header->config_size));
    } else {
        std::vector<u8> data(cache_header->config_size);
        file->Read(data.data(), config_offset, cache_header->config_size);
        ASSERT(disk_cache_config.ParseFromArray(data.data(), data.size()));
    }
    auto new_conf = std::make_shared<TrackerConfig>();
    new_conf->topic = disk_cache_config.topic();
    new_conf->endpoint = disk_cache_config.endpoint();
    new_conf->access_keyid = disk_cache_config.access_keyid();
    new_conf->access_key_secret = disk_cache_config.access_key_secret();
    new_conf->project = disk_cache_config.project();
    new_conf->log_store = disk_cache_config.log_store();

    ASSERT(new_conf->Hash() == hash);
    this->config = new_conf;
}

void DiskCache::SaveConfig() {
    ASSERT(config && config->Hash() == hash);
    TrackDiskCacheConfig disk_cache_config{};
    disk_cache_config.set_topic(config->topic);
    disk_cache_config.set_endpoint(config->endpoint);
    disk_cache_config.set_access_keyid(config->access_keyid);
    disk_cache_config.set_access_key_secret(config->access_key_secret);
    disk_cache_config.set_project(config->project);
    disk_cache_config.set_log_store(config->log_store);
    auto size = disk_cache_config.ByteSizeLong();

    ASSERT(size <= file->Size() - sizeof(CacheHeader));

    if (IsMapped()) {
        auto memory = reinterpret_cast<u8*>(cache_header) + sizeof(CacheHeader);
        ASSERT(disk_cache_config.SerializeToArray(memory, size));
        cache_header->config_size = size;
        cache_header->config_hash = hash;
        ASSERT(file->Commit());
    } else {
        std::vector<u8> memory(size);
        ASSERT(disk_cache_config.SerializeToArray(memory.data(), size));
        ASSERT(file->Write(memory.data(), sizeof(CacheHeader), size));
        cache_header->config_size = size;
        cache_header->config_hash = hash;
        ASSERT(file->Write(*cache_header));
        ASSERT(file->Flush());
    }
}

DiskCache::~DiskCache() {
    if (IsMapped()) {
        Unmap(cache_header, file->Size());
    }
}

}  // namespace tapsdk::tracker