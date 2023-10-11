//
// Created by 甘尧 on 2023/9/14.
//

#pragma once

#include <span>
#include <atomic>
#include "base/types.h"
#include "sdk/platform.h"
#include "sdk/tapsdk.h"
#include "track_msg_cache.pb.h"
#include "google/protobuf/arena.h"

namespace tapsdk::tracker {

class TrackMessageImpl : public TrackMessage {
public:
    explicit TrackMessageImpl(const std::string& topic);

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