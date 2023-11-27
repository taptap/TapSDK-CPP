# TapSDK

TapSDK cpp

# Build

## 依赖

brew install swig

## 编译脚本

/scripts 下选择对应平台的脚本运行

## 开发

Clion / VS 2022 可以直接打开本项目

# Binding

## Java

-DJAVA_BINDING=${生成 Java 的目标路径}

## C#

-DCSHARP_BINDING=${生成 C# 的目标路径}

# 接入

## C++ 内部接口

### 实现平台相关接口

/source/sdk/platform.h
实现平台 Device 并调用 Device.SetCurrent

### 调用初始化

```c++
    tapsdk::Config config {
            .enable_tap_login = true,
            .enable_duration_statistics = true,
            .enable_tap_tracker = true,
            .process_name = "main_process"
    };
    tapsdk::Init(config);
``` 

### 设置当前游戏信息

/source/sdk/tapsdk.h
实现 Game 并调用 Game.SetCurrent

### 设置当前用户信息

/source/sdk/tapsdk.h
实现 TDSUser 并调用 TDSUser.SetCurrent

### 前后台事件

- Device.OnBackground
  当 App 进入后台时调用
- Device.OnForeground
  当 App 进入前台时调用

## C 公开接口

### 返回值

```c++
  enum tapsdk_result {
      TAPSDK_SUCCESS = 0,
      TAPSDK_TIMEOUT = 1,
      TAPSDK_NO_INIT = 2,
      TAPSDK_ERROR = 3,
      TAPSDK_INVALID_INPUT = 4
  };
```

### 初始化

```c++
    auto path = std::filesystem::current_path();
    // 设备相关信息
    tapsdk_device device {
            .device_id = "test_device_id",
            .cache_dir = path.c_str(),
    };
    tapsdk_config config {
            .enable_duration_statistics = true,
            .enable_tap_tracker = true,
            .region = TDS_REGION_GLOBAL,
            .process_name = "main",
            .device = &device
    };
    assert(tapsdk_init(&config) == TAPSDK_SUCCESS);
```

### 设置游戏信息

```c++
    tapsdk_game game {
            .client_id = "0RiAlMny7jiz086FaU",
            .identify = "com.test.game"
    };
    assert(tapsdk_game_set(&game) == TAPSDK_SUCCESS);
```

### 设置用户信息

```c++
    tapsdk_user user {
            .user_id = "{\n"
                    "    \"tds_id\":\"xxxx\",\n"
                    "    \"open_id\":\"xxxx\"\n"
                    "}",
            .contain_tap_info = false
    };
    assert(tapsdk_user_set(&user) == TAPSDK_SUCCESS);
```

### 前后台事件

- tapsdk_window_on_foreground()
  当 App 进入后台时调用
- tapsdk_window_on_background()
  当 App 进入前台时调用

### Tap Tracker
#### 根据配置创建 Message
```c++
    tapsdk_tracker_config tracker_config {};
    tracker_config.topic = "tds_topic";
    tracker_config.endpoint = "openlog.xdrnd.com";
    tracker_config.access_keyid = "${You ID}";
    tracker_config.access_key_secret = "${You Key}";
    tracker_config.project = "tds";
    tracker_config.log_store = "tapsdk_us";

    tapsdk_tracker_message *message;
    assert(tapsdk_tracker_create(&tracker_config, &message) == TAPSDK_SUCCESS);
```
#### 添加 Message 字段
```c++
    assert(tapsdk_tracker_msg_add_param(message, "test_key", "test_value") == TAPSDK_SUCCESS);
    assert(tapsdk_tracker_msg_add_content(message, "test_key", "test_value") == TAPSDK_SUCCESS);
```
#### 提交 Message
```c++
    assert(tapsdk_tracker_flush(message) == TAPSDK_SUCCESS);
```