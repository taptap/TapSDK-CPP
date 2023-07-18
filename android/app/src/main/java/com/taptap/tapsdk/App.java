package com.taptap.tapsdk;

import android.app.Application;

import com.taptap.tapsdk.bindings.java.Game;
import com.taptap.tapsdk.bindings.java.TDSUser;

public class App extends Application {

    @Override
    public void onCreate() {
        super.onCreate();
        try {
            TapSDK.initSDK(this);
            TDSUser.SetCurrent(new TDSUser("test_id") {
                @Override
                public boolean ContainTapInfo() {
                    return false;
                }
            });
            Game.SetCurrent(new Game() {
                @Override
                public String GetGameID() {
                    return "test_game_id";
                }

                @Override
                public String GetPackageName() {
                    return "test_game_pkg";
                }
            });
        } catch (Throwable e) {
            throw new RuntimeException(e);
        }
    }
}
