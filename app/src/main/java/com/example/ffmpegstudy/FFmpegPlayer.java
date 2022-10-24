package com.example.ffmpegstudy;

import android.view.Surface;

/**
 * @ClassName: FFmpegPlayer
 * @Description: java类作用描述
 * @Author: liyihuan
 * @Date: 2022/10/20 22:46
 */
class FFmpegPlayer {

    // Used to load the 'ffmpegstudy' library on application startup.
    static {
        System.loadLibrary("ffmpegstudy");
    }

    public static final int MSG_DECODER_INIT_ERROR      = 0;
    public static final int MSG_DECODER_READY           = 1;
    public static final int MSG_DECODER_DONE            = 2;
    public static final int MSG_REQUEST_RENDER          = 3;
    public static final int MSG_DECODING_TIME           = 4;

    public static final int VIDEO_RENDER_OPENGL         = 0;
    public static final int VIDEO_RENDER_ANWINDOW       = 1;
    public static final int VIDEO_RENDER_3D_VR          = 2;


    public interface EventCallback {
        void onPlayerEvent(int msgType, float msgValue);
    }

    private EventCallback mEventCallback = null;

    public void addEventCallback(EventCallback callback) {
        mEventCallback = callback;
    }

    private void playerEventCallback(int msgType, float msgValue) {
        if(mEventCallback != null)
            mEventCallback.onPlayerEvent(msgType, msgValue);

    }

    /**
     * A native method that is implemented by the 'ffmpegstudy' native library,
     * which is packaged with this application.
     */
    public native String native_helloFFmpeg();

    public native void native_init(int mediaType, String url, Surface surfaceView);
    public native void native_play();




    public static native void native_OnSurfaceCreated(int renderType);
    public static native void native_OnSurfaceChanged(int renderType, int width, int height);
    public static native void native_OnDrawFrame(int renderType);
}
