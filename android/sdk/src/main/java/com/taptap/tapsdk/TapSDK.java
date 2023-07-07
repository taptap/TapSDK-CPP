package com.taptap.tapsdk;

import android.app.Activity;
import android.app.Application;
import android.content.Context;
import android.os.Bundle;
import com.taptap.tapsdk.bindings.java.Window;

import androidx.annotation.NonNull;
import androidx.annotation.Nullable;

import java.util.concurrent.atomic.AtomicInteger;

public class TapSDK {

    private static final AtomicInteger visibleActivities = new AtomicInteger(0);

    public static void initSDK(Context context) throws Throwable {
        System.loadLibrary("bindings-java");
        Application application = (Application) context.getApplicationContext();
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
