package com.example.ffmpegstudy;

import android.app.Application;
import android.os.Build;
import android.os.FileUtils;
import android.os.StrictMode;

/**
 * @author liyihuan
 * @date 2022/11/25
 * @description
 */
public class BaseApplication extends Application {
    private static BaseApplication mInstance;

    @Override
    public void onCreate() {
        mInstance = this;
        Constant.copyAssets();
        super.onCreate();
    }

    public static Application getAppContext() {
        return mInstance;
    }

}
