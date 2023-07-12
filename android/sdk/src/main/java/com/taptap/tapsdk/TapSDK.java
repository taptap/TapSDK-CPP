package com.taptap.tapsdk;

import android.annotation.SuppressLint;
import android.app.Activity;
import android.app.Application;
import android.content.Context;
import android.os.Bundle;
import android.provider.Settings;

import com.taptap.tapsdk.bindings.java.Config;
import com.taptap.tapsdk.bindings.java.Game;
import com.taptap.tapsdk.bindings.java.Window;
import com.taptap.tapsdk.bindings.java.Device;

import androidx.annotation.NonNull;
import androidx.annotation.Nullable;

import java.io.File;
import java.util.concurrent.atomic.AtomicInteger;

public class TapSDK {

    private static final AtomicInteger visibleActivities = new AtomicInteger(0);
    private static Device device;

    public static void initSDK(Context context) throws Throwable {
        System.loadLibrary("bindings-java");
        Application application = (Application) context.getApplicationContext();
        device = new Device() {
            @SuppressLint("HardwareIds")
            @Override
            public String GetDeviceID() {
                return Settings.Secure.getString(application.getContentResolver(), Settings.System.ANDROID_ID);
            }
            @Override
            public String GetCacheDir() {
                File dir = new File(application.getDataDir(), "tapsdk");
                if (!dir.exists()) {
                    dir.mkdirs();
                }
                return dir.getAbsolutePath();
            }
            @Override
            public String GetCaCertDir() {
                return "/system/etc/security/cacerts";
            }
        };
        Device.SetCurrent(device);
        Config config = new Config();
        config.setEnable_duration_statistics(true);
        com.taptap.tapsdk.bindings.java.TapSDK.Init(config);
        application.registerActivityLifecycleCallbacks(new Application.ActivityLifecycleCallbacks() {
            @Override
            public void onActivityCreated(@NonNull Activity activity, @Nullable Bundle savedInstanceState) {

            }

            @Override
            public void onActivityStarted(@NonNull Activity activity) {

            }

            @Override
            public void onActivityResumed(@NonNull Activity activity) {
                if (visibleActivities.incrementAndGet() == 1) {
                    Window.OnForeground();
                }
            }

            @Override
            public void onActivityPaused(@NonNull Activity activity) {
                if (visibleActivities.decrementAndGet() == 0) {
                    Window.OnBackground();
                }
            }

            @Override
            public void onActivityStopped(@NonNull Activity activity) {

            }

            @Override
            public void onActivitySaveInstanceState(@NonNull Activity activity, @NonNull Bundle outState) {

            }

            @Override
            public void onActivityDestroyed(@NonNull Activity activity) {

            }
        });
    }
}
