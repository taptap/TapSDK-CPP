//
// Created by 甘尧 on 2023/11/14.
//

#ifndef TAPSDK_API_H
#define TAPSDK_API_H

#include "stddef.h"
#include "stdint.h"

#ifdef WIN32
#define TDS_API __declspec(dllexport)
#else
#define TDS_API
#endif

#define TDS_DEV_TYPE_LOCAL 0
#define TDS_DEV_TYPE_SANDBOX 1
#define TDS_DEV_TYPE_CLOUD 2

#define TDS_REGION_CN 0
#define TDS_REGION_GLOBAL 1
#define TDS_REGION_RND 2

#if __cplusplus
extern "C" {
#endif

enum tapsdk_result {
    TAPSDK_SUCCESS = 0,
    TAPSDK_TIMEOUT = 1,
    TAPSDK_NO_INIT = 2,
    TAPSDK_ERROR = 3,
    TAPSDK_INVALID_INPUT = 4
};

struct TDS_API tapsdk_device_info {
    const char* device_version = NULL;
    const char* model = NULL;
    const char* platform = NULL;
    const char* engine = NULL;
    const char* os_version = NULL;
    const char* android_id = NULL;
    const char* ram_size = NULL;
    const char* rom_size = NULL;
    const char* network_type = NULL;
    const char* mobile_type = NULL;
    const char* cpu_info = NULL;
};

struct TDS_API tapsdk_device {
    int type = TDS_DEV_TYPE_LOCAL;
    const char* device_id = NULL;
    const char* cache_dir = NULL;
    tapsdk_device_info* info = NULL;
};

struct TDS_API tapsdk_config {
    bool enable_tap_login = false;
    bool enable_duration_statistics = false;
    bool enable_tap_tracker = false;
    int region = TDS_REGION_CN;
    const char* client_id = NULL;
    const char* process_name = NULL;
    const char* sdk_version = NULL;
    uint32_t tracker_group_size = 0;
    tapsdk_device* device = NULL;
};

struct TDS_API tapsdk_tracker_config {
    const char* topic = NULL;
    const char* endpoint = NULL;
    const char* access_keyid = NULL;
    const char* access_key_secret = NULL;
    const char* project = NULL;
    const char* log_store = NULL;
    const char* sdk_version = NULL;
    const char* sdk_version_name = NULL;
    uint64_t hash = 0;
};

struct TDS_API tapsdk_user {
    bool contain_tap_info = false;
    const char* user_id = NULL;
};

struct TDS_API tapsdk_game {
    const char* client_id = NULL;
    const char* identify = NULL;
    const char* app_version = NULL;
};

struct TDS_API tapsdk_tracker_message {};

TDS_API tapsdk_result tapsdk_init(tapsdk_config* config);
TDS_API tapsdk_result tapsdk_game_set(tapsdk_game* game);
TDS_API tapsdk_result tapsdk_user_set(tapsdk_user* user);

TDS_API tapsdk_result tapsdk_window_on_foreground();
TDS_API tapsdk_result tapsdk_window_on_background();

TDS_API tapsdk_result tapsdk_tracker_create(tapsdk_tracker_config* config,
                                            tapsdk_tracker_message** message);
TDS_API tapsdk_result tapsdk_tracker_msg_add_param(tapsdk_tracker_message* message,
                                                   const char* key,
                                                   const char* value);
TDS_API tapsdk_result tapsdk_tracker_msg_add_content(tapsdk_tracker_message* message,
                                                     const char* key,
                                                     const char* value);
TDS_API tapsdk_result tapsdk_tracker_flush(tapsdk_tracker_message* message);

#if __cplusplus
}
#endif

#endif  // TAPSDK_API_H
