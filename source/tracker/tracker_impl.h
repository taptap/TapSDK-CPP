//
// Created by 甘尧 on 2023/9/14.
//

#pragma once

#include <span>
#include <atomic>
#include <list>
#include "base/types.h"
#include "sdk/platform.h"
#include "sdk/tapsdk.h"
#include "net/network.h"
#include "track_msg_cache.pb.h"
#include "google/protobuf/arena.h"

namespace tapsdk::tracker {

class DiskCache;

class TrackerCache {
public:
    explicit TrackerCache(const std::string &path, u64 hash, const std::shared_ptr<TrackerConfig>& config = {});

    bool Push(const std::shared_ptr<TrackMessage> &tracker);

    std::vector<u8> SerializeToUpload();

    net::TapHttpClient &GetHttpClient();

    std::shared_ptr<TrackerConfig> GetConfig();

    u32 GetCount() const;

    void Clear();

    void Destroy();

private:
    std::mutex lock;
    std::shared_ptr<TrackerConfig> config;
    std::unique_ptr<DiskCache> disk_cache;
    std::unique_ptr<net::TapHttpClient> http_client;
};

class TrackMessageImpl : public TrackMessage {
public:
    explicit TrackMessageImpl(const std::shared_ptr<TrackerConfig> &config);

    void AddContent(const std::string& key, const std::string& value) override;
    void AddParam(const std::string& key, const std::string& value) override;

    std::span<u8> Serialize();
    bool Deserialize(std::span<u8> data);
    void Flushed();

    google::protobuf::RepeatedPtrField<TrackMsgContent> &GetContents();
    google::protobuf::RepeatedPtrField<TrackMsgContent> &GetParams();

private:
    std::atomic<bool> dirty{false};
    std::atomic<bool> flushed{false};
    std::vector<u8> data{};
    TrackMsgCacheProto proto{};
};

}