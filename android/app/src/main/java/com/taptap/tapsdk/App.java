package com.taptap.tapsdk;

import android.app.Application;

public class App extends Application {

    @Override
    public void onCreate() {
        super.onCreate();
        try {
            TapSDK.initSDK(this);
        } catch (Throwable e) {
            throw new RuntimeException(e);
        }
    }
}
