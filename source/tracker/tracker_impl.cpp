//
// Created by 甘尧 on 2023/9/14.
//

#include <bitset>
#include <list>
#include <map>
#include <mutex>
#include <atomic>
#include <unordered_map>
#include "base/base64.h"
#include "base/hmac.h"
#include "base/logging.h"
#include "base/md5.h"
#include "base/sha1.h"
#include "base/uuid.h"
#include "ghc/fs_std_select.hpp"
#include "lz4.h"
#include "net/network.h"
#include "track_log_bean.pb.h"
#include "tracker/cache.h"
#include "tracker/model.h"
#include "tracker/tracker_impl.h"
#include "core/runtime.h"

namespace tapsdk::tracker {

using namespace com::tds::common::tracker::entities;

constexpr auto header_fmt =
        "POST\n"
        "{}\n"
        "application/x-protobuf\n"
        "x-log-apiversion:0.6.0\n"
        "x-log-bodyrawsize:{}\n"
        "x-log-compresstype:lz4\n"
        "x-log-signaturemethod:hmac-sha1\n"
        "x-log-timestamp:{}\n"
        "/putrecords/{}/{}";

constexpr auto TAP_TRACKER_VERSION = "1.0.1";
constexpr auto MAX_CACHE_COUNT = 16;
constexpr auto UPLOAD_TRACKER_COUNT = 32;
constexpr auto check_ms = Ms(20 * 1000);
using TrackerCacheIds = std::bitset<MAX_CACHE_COUNT>;

static u32 tracker_group_size;
static std::mutex tracker_lock;
static std::atomic<bool> inited{false};
static std::atomic<bool> uploading{false};
static fs::path tracker_cache;
static std::unordered_map<u64, std::map<u32, std::shared_ptr<TrackerCache>>> trackers_cache{};
static std::unordered_map<u64, TrackerCacheIds> tracker_cache_ids{};
static std::shared_ptr<Event> check_upload_event;
static std::atomic<TrackerCache *> latest_upload_failed;

static std::string HashHmac(const std::string& content, const std::string& key) {
    std::string sha1hmac = hmac<SHA1>(content, key);
    string bin_dig, base64_str;
    HexToBin(sha1hmac, bin_dig);
    BinToBase64(bin_dig, base64_str);
    return std::move(base64_str);
}

TrackMessageImpl::TrackMessageImpl(const std::shared_ptr<TrackerConfig>& config)
        : TrackMessage(config) {
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

size_t TrackMessageImpl::GetSerializeSize() {
    return proto.ByteSizeLong();
}

bool TrackMessageImpl::SerializeToBuffer(std::span<u8> buffer) {
    auto serialize_size = GetSerializeSize();
    if (buffer.size() <serialize_size) {
        return false;
    }
    return proto.SerializeToArray(buffer.data(), serialize_size);
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

TrackerCache::TrackerCache(const std::string& path,
                           u64 hash,
                           const std::shared_ptr<TrackerConfig>& config)
        : config(config) {
    disk_cache = std::make_unique<DiskCache>(hash, path, config);
    disk_cache->Init();
    this->config = disk_cache->GetConfig();
    http_client = net::CreateHttpClient(this->config->endpoint.c_str(), true);
    http_client->SetResultUnwrap([](auto response) -> auto { return net::ResultWrapper{0, ""}; });
}

bool TrackerCache::Push(const std::shared_ptr<TrackMessage>& tracker) {
    auto tracker_impl = std::dynamic_pointer_cast<TrackMessageImpl>(tracker);
    std::scoped_lock guard(lock);
    try {
        return disk_cache->Push(*tracker_impl);
    } catch (...) {
        return false;
    }
}

std::vector<u8> TrackerCache::SerializeToUpload() {
    LogGroup group;
    auto trackers = disk_cache->Load();
    if (trackers.empty()) {
        return {};
    }
    group.set_topic(config->topic);
    for (auto& track : trackers) {
        auto log = group.add_logs();
        log->set_time(track.GetCreateTime());
        // Contents
        for (auto& itr : track.GetContents()) {
            auto log_cnt = log->add_contents();
            log_cnt->set_key(itr.key());
            log_cnt->set_value(itr.value());
        }
        // Params
        for (auto& itr : track.GetParams()) {
            auto log_cnt = log->add_contents();
            log_cnt->set_key(itr.key());
            log_cnt->set_value(itr.value());
        }
    }
    auto proto_size = group.ByteSizeLong();
    std::vector<u8> data(proto_size);
    group.SerializeToArray(data.data(), proto_size);
    return data;
}

net::TapHttpClient& TrackerCache::GetHttpClient() { return *http_client; }

std::shared_ptr<TrackerConfig> TrackerCache::GetConfig() { return config; }

u32 TrackerCache::GetCount() const { return disk_cache->GetCount(); }

void TrackerCache::Clear() {
    disk_cache->Clear();
}

void TrackerCache::Destroy() {
    disk_cache->Destroy();
}

static std::string DiskCachePath(u64 hash, u32 index) {
    return (tracker_cache / fmt::format("{}@{}.trc", hash, index)).string();
}

static void FillCommons(TrackMessageImpl& msg, TrackerConfig& config) {
    auto device = platform::Device::GetCurrent();
    auto game = Game::GetCurrent();
    auto device_info = device->GetDeviceInfo();
    msg.AddParam(ReportKey::SDK_VERSION, fmt::format("{}", config.sdk_version));
    msg.AddParam(ReportKey::SDK_VERSION_NAME, config.sdk_version_name);
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

static net::ResultAsync<std::shared_ptr<UploadResult>> UploadTopicTrackers(
        std::shared_ptr<TrackerCache> trackers) {
    auto serialize_buffer = trackers->SerializeToUpload();
    auto config = trackers->GetConfig();
    auto& http_client = trackers->GetHttpClient();
    if (serialize_buffer.empty()) {
        co_return net::MakeError(-1, -1, "No buffer!");
    }

    // lz4 compress
    auto compressed_size = LZ4_compressBound(serialize_buffer.size());
    std::vector<u8> compressed_buffer(compressed_size);
    compressed_size = LZ4_compress(reinterpret_cast<const char*>(serialize_buffer.data()),
                                   reinterpret_cast<char*>(compressed_buffer.data()),
                                   serialize_buffer.size());
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
                                  serialize_buffer.size(),
                                  time_stamp,
                                  config->project,
                                  config->log_store);
    auto header_sig_fmt = fmt::format(
            "LOG {}:{}", config->access_keyid, HashHmac(header_sig, config->access_key_secret));

    std::vector<net::Pair> headers{
            {"x-log-timestamp", fmt::format("{}", time_stamp)},
            {"Content-MD5", buffer_md5_string},
            {"Content-Length", fmt::format("{}", compressed_size)},
            {"x-log-bodyrawsize", fmt::format("{}", serialize_buffer.size())},
            {"Authorization", header_sig_fmt},
            {"Content-Type", "application/x-protobuf"},
            {"x-log-apiversion", "0.6.0"},
            {"x-log-compresstype", "lz4"},
            {"x-log-signaturemethod", "hmac-sha1"},
            {"Host", config->endpoint},
            {"accept", "*/*"},
            {"Accept-Encoding", "identity"}};
    auto upload_result = co_await http_client.PostAsync<UploadResult>(
            WebPath("putrecords") / config->project / config->log_store,
            headers,
            {},
            {(char*)compressed_buffer.data(), compressed_buffer.size()});

    co_return upload_result;
}

void SyncLoadTrackersCache() {
    fs::directory_iterator list{};
    try {
        list = fs::directory_iterator(tracker_cache);
    } catch (...) {
        LOG_DEBUG("No cache files!");
    }
    for (auto& f : list) {
        auto f_name = f.path().filename().string();
        auto is_dir = f.is_directory();
        if (is_dir) continue;

        auto trc_index = f_name.find_first_of(".trc");
        auto is_trc = trc_index == (f_name.length() - 4);
        auto filename = f_name.substr(0, trc_index);

        if (is_trc) {
            std::scoped_lock guard(tracker_lock);
            auto at_index = filename.find_first_of('@');
            if (at_index <= 0) {
                continue;
            }
            auto hash_str = filename.substr(0, at_index);
            auto idx_str = filename.substr(at_index + 1, filename.length());

            try {
                u64 hash = std::stoull(hash_str);
                u32 idx = std::stoul(idx_str);
                auto &caches = trackers_cache[hash];
                auto &ids = tracker_cache_ids[hash];
                caches.emplace(idx, std::make_shared<TrackerCache>(f.path().string(), hash));
                ids[idx] = true;
            } catch (...) {
                LOG_ERROR("Load tracker cache {} error!", f.path().string());
            }
        }
    }
}

static void UploadTrackerCacheAsync(u64 hash, u32 index) {
    if (uploading) {
        return;
    }
    auto &caches = trackers_cache[hash];
    auto itr = caches.find(index);
    if (itr == caches.end()) {
        return;
    }
    auto cache = itr->second;
    caches.erase(itr);
    uploading = true;
    UploadTopicTrackers(cache).start([hash, index, cache] (auto result) {
        bool success{false};
        if (!result.hasError()) {
            success = result.value().has_value();
        }
        std::scoped_lock guard(tracker_lock);
        if (success) {
            try {
                cache->Clear();
                trackers_cache[hash].emplace(index, cache);
            } catch (...) {
                cache->Destroy();
                tracker_cache_ids[hash][index] = false;
            }
        } else {
            trackers_cache[hash].emplace(index, cache);
            latest_upload_failed = cache.get();
        }
        uploading = false;
    });
}

static void CheckAndUploadTrackers() {
    std::scoped_lock guard(tracker_lock);
    if (uploading) {
        return;
    }
    struct {
        u64 hash = 0;
        u32 index = 0;
    } cache_upload{};
    for (auto& [hash, caches] : trackers_cache) {
        for (auto [id, cache] : caches) {
            if (cache->GetCount() > 0) {
                cache_upload.hash = hash;
                cache_upload.index = id;
                if (latest_upload_failed != cache.get()) {
                    continue;
                }
                goto do_upload;
            }
        }
    }
do_upload:
    if (cache_upload.hash) {
        UploadTrackerCacheAsync(cache_upload.hash, cache_upload.index);
    }
}

static std::shared_ptr<TrackerCache> CreateCache(u64 hash, u32 index, const std::shared_ptr<TrackerConfig> &config) {
    auto path = DiskCachePath(hash, index);
    try {
        auto new_cache = std::make_shared<TrackerCache>(path, hash, config);
        tracker_cache_ids[hash][index] = true;
        trackers_cache[hash].emplace(index, new_cache);
        return new_cache;
    } catch (...) {
        File::Delete(path);
        LOG_ERROR("TrackerCache {} error!", path);
        return {};
    }
}

void Init(const Config& config) {
    tracker_group_size = config.tracker_group_size ? config.tracker_group_size : 2000;
    auto dev = platform::Device::GetCurrent();
    ASSERT(dev);
    tracker_cache = fs::path(dev->GetCacheDir()) / "tap_trackers" / config.process_name;
    check_upload_event = Event::Create([] (Duration later, std::uintptr_t user_data) {
        CheckAndUploadTrackers();
        Runtime::Get().Timer().PostEvent(check_upload_event, check_ms);
    }, "Check Upload");
    SyncLoadTrackersCache();
    Runtime::Get().Timer().PostEvent(check_upload_event);
    inited = true;
}

std::shared_ptr<TrackMessage> CreateTracker(const std::shared_ptr<TrackerConfig>& config) {
    if (!inited || !config) {
        return {};
    }
    auto tracker = std::make_shared<TrackMessageImpl>(config);
    FillCommons(*tracker, *config);
    return tracker;
}

bool FlushTracker(const std::shared_ptr<TrackMessage>& tracker) {
    if (!inited || !tracker) {
        return false;
    }
    auto tracker_impl = std::dynamic_pointer_cast<TrackMessageImpl>(tracker);
    tracker_impl->Flushed();
    auto config_hash = tracker->GetConfig()->Hash();
    std::scoped_lock guard(tracker_lock);
    auto& caches = trackers_cache[config_hash];
    bool pushed{false};
    for (auto& [index, cache] : caches) {
        if (cache->GetCount() >= tracker_group_size) {
            UploadTrackerCacheAsync(config_hash, index);
            continue;
        }
        pushed = cache->Push(tracker);
        if (pushed) {
            if (cache->GetCount() >= UPLOAD_TRACKER_COUNT) {
                UploadTrackerCacheAsync(config_hash, index);
            }
            break;
        }
    }
    if (!pushed) {
        auto& ids = tracker_cache_ids[config_hash];
        for (int i = 0; i < MAX_CACHE_COUNT; ++i) {
            if (!ids[i]) {
                if (auto new_cache = CreateCache(config_hash, i, tracker->GetConfig()); new_cache) {
                    pushed = new_cache->Push(tracker);
                    break;
                }
            }
        }
    }
    return pushed;
}

}  // namespace tapsdk::tracker