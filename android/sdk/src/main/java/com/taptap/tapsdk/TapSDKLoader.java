package com.taptap.tapsdk;

public class TapSDKLoader {
    public static void load() throws Throwable {
        System.loadLibrary("bindings-java");
    }
}
