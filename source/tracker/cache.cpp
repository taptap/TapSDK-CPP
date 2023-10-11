//
// Created by 甘尧 on 2023/9/25.
//

#include <utility>
#include "base/alignment.h"
#include "base/logging.h"
#include "cache.h"
#include "tracker_impl.h"
#include "unistd.h"

#ifdef __APPLE__
#include <libkern/OSCacheControl.h>
#endif

namespace tapsdk::tracker {

constexpr std::array<u8, 4> cache_magic = {'t', 'd', 's', 'c'};
constexpr auto cache_version_code = 1;
constexpr auto max_track_counts = 5000;
constexpr auto index_file_size = sizeof(CacheIndex) + max_track_counts * sizeof(CacheEntry);
constexpr auto max_cache_size = 1024 * 1024 * 2;
const static auto cache_idx_size = AlignUp(index_file_size, getpagesize());

static void ClearDCache(void* start, size_t size) {
#ifdef __APPLE__
    sys_dcache_flush(reinterpret_cast<char*>(start), size);
#endif
}

bool CacheToDisk(const std::string& topic, const std::list<std::shared_ptr<TrackMessage>>& msgs) {
    return true;
}

TrackCache::TrackCache(std::string topic, std::string path)
        : topic(std::move(topic)), path(std::move(path)) {
    index_file = std::make_unique<File>(this->path + ".idx");
    content_file = std::make_unique<File>(this->path + ".cnt");
}

void TrackCache::Init() {
    ASSERT_MSG(index_file->Open(FileAccessMode::ReadWrite),
               "Index file {} open failed!",
               index_file->GetPath());
    ASSERT_MSG(content_file->Open(FileAccessMode::ReadWrite),
               "Content file {} open failed!",
               index_file->GetPath());

    auto should_init{false};

    if (index_file->Size() != cache_idx_size) {
        ASSERT(index_file->Resize(AlignUp(index_file_size, getpagesize())));
        should_init = true;
    }

    if (content_file->Size() != max_cache_size) {
        ASSERT(content_file->Resize(max_cache_size));
        should_init = true;
    }

    auto index_file_mem = index_file->Map(0, cache_idx_size);
    auto content_file_mem = content_file->Map(0, max_cache_size);
    auto use_mmap = index_file_mem && content_file_mem;

    if (use_mmap) {
        index_header = reinterpret_cast<CacheIndex*>(index_file_mem);
        auto entry_start = reinterpret_cast<u8*>(index_file_mem) + sizeof(CacheIndex);
        entries = {reinterpret_cast<CacheEntry*>(entry_start), max_track_counts};
        entries_header = reinterpret_cast<CacheEntries*>(content_file_mem);
    } else {
        index_header = &idx_header_buf;
        entries_header = &cnt_header_buf;
        *entries_header = content_file->Read<CacheEntries>(0);
        *index_header = index_file->Read<CacheIndex>(0);
    }

    if (should_init || index_header->magic != cache_magic || entries_header->magic != cache_magic) {
        entries_header->ver_code = cache_version_code;
        entries_header->magic = cache_magic;
        index_header->ver_code = cache_version_code;
        index_header->magic = cache_magic;

        entries_header->size = 0;
        index_header->entry_size = 0;

        if (use_mmap) {
            ASSERT(content_file->Commit());
            ASSERT(index_file->Commit());
        } else {
            content_file->Write(*entries_header, 0);
            index_file->Write(*index_header, 0);
            ASSERT(content_file->Flush());
            ASSERT(index_file->Flush());
        }
    }
}

bool TrackCache::Push(u64 time, std::span<u8> content) {
    if (content.size() > (max_cache_size - sizeof(CacheEntries) - entries_header->size)) {
        return false;
    }
    u32 cache_offset = sizeof(CacheEntries);
    if (!entries.empty()) {
        if (index_header->entry_size > 0) {
            auto& last_entry = entries[index_header->entry_size - 1];
            cache_offset = last_entry.offset + last_entry.length;
        }
        std::memcpy(reinterpret_cast<u8*>(entries_header) + cache_offset,
                    content.data(),
                    content.size());
        auto& entry = entries[index_header->entry_size];
        entry.offset = cache_offset;
        entry.length = content.size();
        entry.time = time;
        index_header->entry_size++;
        entries_header->size += content.size();
        return content_file->Commit() && index_file->Commit();
    } else {
        if (index_header->entry_size > 0) {
            auto last_entry = index_file->Read<CacheEntry>(
                    sizeof(CacheIndex) + (index_header->entry_size - 1) * sizeof(CacheEntry));
            cache_offset = last_entry.offset + last_entry.length;
        }
        CacheEntry entry{};
        entry.offset = cache_offset;
        entry.length = content.size();
        entry.time = time;
        if (!content_file->Write((void*)content.data(), entry.offset, entry.length)) {
            return false;
        }
        if (!index_file->Write(
                    entry, sizeof(CacheIndex) + sizeof(CacheEntry) * index_header->entry_size)) {
            return false;
        }
        index_header->entry_size++;
        entries_header->size += content.size();
        if (!content_file->Write(entries_header->size, offsetof(CacheEntries, size))) {
            return false;
        }
        if (!index_file->Write(index_header->entry_size, offsetof(CacheIndex, entry_size))) {
            return false;
        }
        return content_file->Flush() && index_file->Flush();
    }
}

std::list<std::shared_ptr<TrackMessage>> TrackCache::Load() {
    std::list<std::shared_ptr<TrackMessage>> res{};
    ASSERT(index_header->entry_size <= max_track_counts);
    if (!entries.empty()) {
        for (u32 i = 0; i < index_header->entry_size; ++i) {
            auto& entry = entries[i];
            ASSERT(entry.offset + entry.length < max_cache_size);
            std::span<u8> data{reinterpret_cast<u8*>(entries_header) + entry.offset, entry.length};
            auto tracker = std::make_shared<TrackMessageImpl>(topic);
            if (tracker->Deserialize(data)) {
                res.push_back(tracker);
            }
        }
    } else {
        std::vector<CacheEntry> cache_entries(index_header->entry_size);
        ASSERT(index_file->Read(cache_entries.data(),
                                sizeof(CacheIndex),
                                sizeof(CacheEntry) * index_header->entry_size));
        for (u32 i = 0; i < index_header->entry_size; ++i) {
            auto& entry = cache_entries[i];
            ASSERT(entry.offset + entry.length < max_cache_size);
            std::vector<u8> data(entry.length);
            ASSERT(content_file->Read(data.data(), entry.offset, entry.length));
            auto tracker = std::make_shared<TrackMessageImpl>(topic);
            if (tracker->Deserialize(data)) {
                res.push_back(tracker);
            }
        }
    }
    return res;
}

void TrackCache::Clear() {
    u32 zero = 0;
    content_file->Write(zero, offsetof(CacheEntries, size));
    index_file->Write(zero, offsetof(CacheIndex, entry_size));
    content_file->Flush();
    index_file->Flush();
}

TrackCache::~TrackCache() {
    if (index_header) {
        Unmap(index_header, index_file_size);
    }
    if (entries_header) {
        Unmap(entries_header, max_cache_size);
    }
}

}  // namespace tapsdk::tracker