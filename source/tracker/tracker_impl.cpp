//
// Created by 甘尧 on 2023/9/14.
//

#include <list>
#include <shared_mutex>
#include <unordered_map>
#include "base/base64.h"
#include "base/hmac.h"
#include "base/logging.h"
#include "base/md5.h"
#include "base/sha1.h"
#include "lz4.h"
#include "net/network.h"
#include "track_log_bean.pb.h"
#include "tracker/model.h"
#include "tracker/tracker_impl.h"
#include "tracker/cache.h"
#include "base/uuid.h"
#include "ghc/fs_std_select.hpp"

namespace tapsdk::tracker {

using namespace com::tds::common::tracker::entities;
using TopicTrackers = std::list<std::shared_ptr<TrackMessageImpl>>;

constexpr auto header_fmt = "POST\n"
        "{}\n"
        "application/x-protobuf\n"
        "x-log-apiversion:0.6.0\n"
        "x-log-bodyrawsize:{}\n"
        "x-log-compresstype:lz4\n"
        "x-log-signaturemethod:hmac-sha1\n"
        "x-log-timestamp:{}\n"
        "/putrecords/{}/{}";

constexpr auto TAP_TRACKER_VERSION = "1.0.1";

static std::shared_mutex tracker_lock;
static TrackerConfig tracker_config;
static fs::path tracker_cache;
static std::unordered_map<std::string, TopicTrackers> flushed_trackers{};
static std::unordered_map<std::string, TrackCache> topic_disk_caches{};
static std::unique_ptr<net::TapHttpClient> http_client{};

static std::string HashHmac(const std::string& content, const std::string& key) {
    std::string sha1hmac = hmac<SHA1>(content, key);
    string bin_dig, base64_str;
    HexToBin(sha1hmac, bin_dig);
    BinToBase64(bin_dig, base64_str);
    return std::move(base64_str);
}

TrackMessageImpl::TrackMessageImpl(const std::string& topic)
        : TrackMessage(topic) {
    create_time = std::time(nullptr);
}

void TrackMessageImpl::AddContent(const std::string& key, const std::string& value) {
    ASSERT(!flushed);
    auto cnt = proto.add_log_contents();
    cnt->set_key(key);
    cnt->set_value(value);
    dirty = true;
}

void TrackMessageImpl::AddParam(const std::string& key, const std::string& value) {
    ASSERT(!flushed);
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

void TrackMessageImpl::Flushed() { flushed = true; }

static void FillCommons(TrackMessageImpl &msg) {
    auto device = platform::Device::GetCurrent();
    auto game = Game::GetCurrent();
    auto device_info = device->GetDeviceInfo();
    msg.AddParam(ReportKey::SDK_VERSION, fmt::format("{}", tracker_config.sdk_version));
    msg.AddParam(ReportKey::SDK_VERSION_NAME, tracker_config.sdk_version_name);
    msg.AddParam(ReportKey::DEVICE_ID, device->GetDeviceID());
    msg.AddParam(ReportKey::T_LOG_ID, CreateUUID());
    msg.AddParam(ReportKey::VERSION, TAP_TRACKER_VERSION);
    msg.AddParam(ReportKey::DEVICE_VERSION, device_info->device_version);
    msg.AddParam(ReportKey::MODEL, device_info->model);
    msg.AddParam(ReportKey::CPU, device_info->cpu_info);
    msg.AddParam(ReportKey::APP_PACKAGE_NAME, game->GetPackageName());
    msg.AddParam(ReportKey::APP_VERSION, "");
    msg.AddParam(ReportKey::RAM, device_info->ram_size);
    msg.AddParam(ReportKey::ROM, device_info->rom_size);
    msg.AddParam(ReportKey::NETWORK_TYPE, device_info->network_type);
    msg.AddParam(ReportKey::MOBILE_TYPE, device_info->mobile_type);
    msg.AddParam(ReportKey::OS_PARAM, device_info->platform);
    msg.AddParam(ReportKey::SYSTEM_VERSION, device_info->os_version);
}

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
    if (!serialize_buffer.has_value()) {
        co_return net::MakeError(serialize_buffer.error());
    }

    // lz4 compress
    auto compressed_size = LZ4_compressBound(serialize_buffer->size());
    std::vector<u8> compressed_buffer(compressed_size);
    compressed_size = LZ4_compress(reinterpret_cast<const char*>(serialize_buffer->data()),
                                   reinterpret_cast<char*>(compressed_buffer.data()),
                                   serialize_buffer->size());
    if (!compressed_size) {
        co_return net::MakeError(-1, -1, "lz4 compress error!");
    }
    compressed_buffer.resize(compressed_size);

    // MD5 Sign
    MD5 buffer_md5(compressed_buffer);
    auto buffer_md5_string = buffer_md5.toStr();
    std::transform(buffer_md5_string.begin(),
                   buffer_md5_string.end(),
                   buffer_md5_string.begin(),
                   ::toupper);

    std::time_t time_stamp = std::time(nullptr);
    auto header_sig = fmt::format(header_fmt,
                                  buffer_md5_string,
                                  serialize_buffer->size(),
                                  time_stamp,
                                  tracker_config.project,
                                  tracker_config.log_store);
    auto header_sig_fmt = fmt::format("LOG {}:{}",
                                      tracker_config.access_keyid,
                                      HashHmac(header_sig, tracker_config.access_key_secret));

    std::vector<net::Pair> headers{
            {"x-log-timestamp", fmt::format("{}", time_stamp)},
            {"Content-MD5", buffer_md5_string},
            {"Content-Length", fmt::format("{}", compressed_size)},
            {"x-log-bodyrawsize", fmt::format("{}", serialize_buffer->size())},
            {"Authorization", header_sig_fmt},
            {"Content-Type", "application/x-protobuf"},
            {"x-log-apiversion", "0.6.0"},
            {"x-log-compresstype", "lz4"},
            {"x-log-signaturemethod", "hmac-sha1"},
            {"Host", tracker_config.endpoint},
            {"accept", "*/*"},
            {"Accept-Encoding", "identity"}};
    auto upload_result = co_await http_client->PostAsync<UploadResult>(
            WebPath("putrecords") / tracker_config.project / tracker_config.log_store,
            headers,
            {},
            {(char*)compressed_buffer.data(), compressed_buffer.size()});

    co_return upload_result;
}

void SyncCacheTrackers(const std::string& topic, const TopicTrackers& trackers) {

}

static net::ResultAsync<std::shared_ptr<UploadResult>> CacheAndUploadTrackers(
        const std::string& topic, const TopicTrackers& trackers) {
    auto result = co_await UploadTopicTrackers(topic, trackers);


    co_return result;
}

void Init(const Config& config) {
    ASSERT(config.tracker_config);
    tracker_config = *config.tracker_config;
    auto dev = platform::Device::GetCurrent();
    ASSERT(dev);
    tracker_cache = dev->GetCacheDir();
    http_client = net::CreateHttpClient(tracker_config.endpoint.c_str(), true);
    http_client->SetResultUnwrap([] (auto response) -> auto {
        return net::ResultWrapper{-1, ""};
    });
}

std::shared_ptr<TrackMessage> CreateTracker(const std::string& topic) {
    auto result = std::make_shared<TrackMessageImpl>(topic);
    FillCommons(*result);
    return result;
}

void FlushTracker(const std::shared_ptr<TrackMessage>& tracker) {
    std::scoped_lock guard(tracker_lock);
    auto tracker_impl = std::dynamic_pointer_cast<TrackMessageImpl>(tracker);
    tracker_impl->Flushed();
    flushed_trackers[tracker->GetTopic()].push_back(tracker_impl);

    if (!topic_disk_caches.contains(tracker->GetTopic())) {
        auto cache_path = tracker_cache / "tds_tracker" / tracker->GetTopic();
        auto [itr, success] = topic_disk_caches.try_emplace(tracker->GetTopic(), tracker->GetTopic(), cache_path);
        if (success) {
            auto &cache = itr->second;
            cache.Init();
            cache.Push(tracker->GetCreateTime(), tracker_impl->Serialize());
        }
    } else {
        auto &cache = topic_disk_caches.find(tracker_impl->GetTopic())->second;
        cache.Push(tracker->GetCreateTime(), tracker_impl->Serialize());
    }
    UploadTopicTrackers(tracker->GetTopic(), flushed_trackers[tracker->GetTopic()])
            .start([](auto result) { auto result_val = result.value(); });
}

}  // namespace tapsdk::tracker