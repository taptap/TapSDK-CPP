//
// Created by 甘尧 on 2023/9/14.
//

#include <list>
#include <shared_mutex>
#include <unordered_map>
#include "base/logging.h"
#include "net/network.h"
#include "track_log_bean.pb.h"
#include "tracker/model.h"
#include "tracker/tracker_impl.h"

namespace tapsdk::tracker {

using namespace com::tds::common::tracker::entities;
using TopicTrackers = std::list<std::shared_ptr<TrackMessageImpl>>;

static std::shared_mutex tracker_lock;
static TrackerConfig tracker_config;
static std::list<std::shared_ptr<TrackMessageImpl>> flushed_trackers{};

TrackMessageImpl::TrackMessageImpl(const std::string& topic)
        : TrackMessage(topic), create_time(std::time(nullptr)) {}

void TrackMessageImpl::AddContent(const std::string& key, const std::string& value) {
    ASSERT(flushed);
    auto cnt = proto.add_log_contents();
    cnt->set_key(key);
    cnt->set_value(value);
    dirty = true;
}

void TrackMessageImpl::AddParam(const std::string& key, const std::string& value) {
    ASSERT(flushed);
    auto cnt = proto.add_log_commons();
    cnt->set_key(key);
    cnt->set_value(value);
    dirty = true;
}

std::span<u8> TrackMessageImpl::Serialize() {
    if (dirty || data.empty()) {
        data.resize(proto.ByteSizeLong());
        proto.SerializeToArray(data.data(), data.size());
        dirty = false;
    }
    return data;
}

google::protobuf::RepeatedPtrField<TrackMsgContent>& TrackMessageImpl::GetContents() {
    return *proto.mutable_log_contents();
}

google::protobuf::RepeatedPtrField<TrackMsgContent>& TrackMessageImpl::GetParams() {
    return *proto.mutable_log_commons();
}

bool TrackMessageImpl::Deserialize(std::span<u8> data_) {
    return proto.ParseFromArray(data_.data(), data_.size());
}

u32 TrackMessageImpl::GetCreateTime() const { return create_time; }

void TrackMessageImpl::Flushed() { flushed = true; }

static net::ResultAsync<std::vector<u8>> SerializeTopicTrackers(const std::string& topic,
                                                                const TopicTrackers& trackers) {
    LogGroup group;
    group.set_topic(topic);
    for (auto& msg : trackers) {
        auto log = group.add_logs();
        log->set_time(msg->GetCreateTime());
        // Contents
        for (auto& itr : msg->GetContents()) {
            auto log_cnt = log->add_contents();
            log_cnt->set_key(itr.key());
            log_cnt->set_value(itr.value());
        }
        // Params
        for (auto& itr : msg->GetParams()) {
            auto log_cnt = log->add_contents();
            log_cnt->set_key(itr.key());
            log_cnt->set_value(itr.value());
        }
    }
    auto proto_size = group.ByteSizeLong();
    std::vector<u8> data(proto_size);
    group.SerializeToArray(data.data(), proto_size);
    co_return data;
}

static net::ResultAsync<std::shared_ptr<UploadResult>> UploadTopicTrackers(
        const std::string& topic, const TopicTrackers& trackers) {
    auto serialize_buffer = co_await SerializeTopicTrackers(topic, trackers);

    auto http_client = net::CreateHttpClient("", true);
    auto upload_result = co_await http_client->PostAsync<UploadResult>("", {}, {});

    co_return upload_result;
}

void Init(const Config& config) {
    ASSERT(config.tracker_config);
    tracker_config = *config.tracker_config;
}

std::shared_ptr<TrackMessage> CreateTracker(const std::string& topic) {
    return std::make_shared<TrackMessageImpl>(topic);
}

void FlushTracker(const std::shared_ptr<TrackMessage>& tracker) {
    auto tracker_impl = std::dynamic_pointer_cast<TrackMessageImpl>(tracker);
    tracker_impl->Flushed();
    flushed_trackers.push_back(tracker_impl);
}

}  // namespace tapsdk::tracker