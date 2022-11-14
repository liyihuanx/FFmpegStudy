package com.example.ffmpegstudy.camera;

import android.opengl.GLSurfaceView;
import android.util.Log;
import android.view.Surface;
import android.view.SurfaceHolder;
import android.view.SurfaceView;

import javax.microedition.khronos.egl.EGLConfig;
import javax.microedition.khronos.opengles.GL10;

import androidx.annotation.NonNull;

/**
 * @ClassName: FFMediaRecord
 * @Description: java类作用描述
 * @Author: liyihuan
 * @Date: 2022/11/7 20:42
 */
public class FFMediaRecord implements GLSurfaceView.Renderer {
    private static final String TAG = "FFMediaRecord";

    public static final int IMAGE_FORMAT_RGBA = 0x01;
    public static final int IMAGE_FORMAT_NV21 = 0x02;
    public static final int IMAGE_FORMAT_NV12 = 0x03;
    public static final int IMAGE_FORMAT_I420 = 0x04;

    public static final int RECORDER_TYPE_SINGLE_VIDEO = 0; //仅录制视频
    public static final int RECORDER_TYPE_SINGLE_AUDIO = 1; //仅录制音频
    public static final int RECORDER_TYPE_AV = 2; //同时录制音频和视频,打包成 MP4 文件


    private GLSurfaceView mGLSurfaceView;
    // 与native的桥梁
    private long mNativeContextHandle;

    static {
        System.loadLibrary("ffmpegstudy");
    }

    public void init(GLSurfaceView surfaceView) {
        if (surfaceView != null) {
            mGLSurfaceView = surfaceView;
            mGLSurfaceView.setEGLContextClientVersion(2);
            mGLSurfaceView.setRenderer(this);
            mGLSurfaceView.setRenderMode(GLSurfaceView.RENDERMODE_WHEN_DIRTY);
        }
        native_CreateContext();
        native_Init();
    }

    public void onPreviewFrame(int format, byte[] data, int width, int height) {
//        Log.d(TAG, "onPreviewFrame() called with: data = [" + data + "], width = [" + width + "], height = [" + height + "]");
        native_OnPreviewFrame(format, data, width, height);
    }


    @Override
    public void onSurfaceCreated(GL10 gl, EGLConfig config) {
        native_OnSurfaceCreated();
    }

    @Override
    public void onSurfaceChanged(GL10 gl, int width, int height) {
        native_OnSurfaceChanged(width, height);
    }

    @Override
    public void onDrawFrame(GL10 gl) {
        native_OnDrawFrame();
    }

    public void requestRender() {
        if (mGLSurfaceView != null) {
            mGLSurfaceView.requestRender();
        }
    }

    public void unInit() {
        native_DestroyContext();
        native_UnInit();
    }

    public void setTransformMatrix(int degree, int mirror) {
        Log.d(TAG, "setTransformMatrix() called with: degree = [" + degree + "], mirror = [" + mirror + "]");
        native_SetTransformMatrix(0, 0, 1, 1, degree, mirror);
    }

    public void startRecord(int recorderType, String outUrl, int frameWidth, int frameHeight, long videoBitRate, int fps) {
        Log.d(TAG, "startRecord() called with: recorderType = [" + recorderType + "], outUrl = [" + outUrl + "], frameWidth = [" + frameWidth + "], frameHeight = [" + frameHeight + "], videoBitRate = [" + videoBitRate + "], fps = [" + fps + "]");
        native_StartRecord(recorderType, outUrl, frameWidth, frameHeight, videoBitRate, fps);
    }

    public void stopRecord() {
        native_StopRecord();
    }


    public void onAudioData(byte[] data, int dataSize) {
        native_onAudioData(data,dataSize);
    }

    protected native void native_CreateContext();

    protected native void native_DestroyContext();

    protected native int native_Init();

    protected native int native_UnInit();

    protected native void native_OnPreviewFrame(int format, byte[] data, int width, int height);

    protected native void native_OnSurfaceCreated();

    protected native void native_OnSurfaceChanged(int width, int height);

    protected native void native_OnDrawFrame();

    protected native void native_SetTransformMatrix(float translateX, float translateY, float scaleX, float scaleY, int degree, int mirror);

    protected native int native_StartRecord(int recorderType, String outUrl, int frameWidth, int frameHeight, long videoBitRate, int fps);

    protected native int native_StopRecord();

    protected native void native_onAudioData(byte[] data, int dataSize);


}
