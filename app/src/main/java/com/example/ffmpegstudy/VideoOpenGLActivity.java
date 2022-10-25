package com.example.ffmpegstudy;

import androidx.appcompat.app.AppCompatActivity;
import androidx.core.app.ActivityCompat;

import android.Manifest;
import android.content.pm.PackageManager;
import android.opengl.GLSurfaceView;
import android.os.Bundle;
import android.util.Log;
import android.view.View;

import javax.microedition.khronos.egl.EGLConfig;
import javax.microedition.khronos.opengles.GL10;

import static com.example.ffmpegstudy.FFmpegPlayer.MSG_DECODER_DONE;
import static com.example.ffmpegstudy.FFmpegPlayer.MSG_DECODER_INIT_ERROR;
import static com.example.ffmpegstudy.FFmpegPlayer.MSG_DECODER_READY;
import static com.example.ffmpegstudy.FFmpegPlayer.MSG_DECODING_TIME;
import static com.example.ffmpegstudy.FFmpegPlayer.MSG_REQUEST_RENDER;
import static com.example.ffmpegstudy.FFmpegPlayer.VIDEO_RENDER_ANWINDOW;
import static com.example.ffmpegstudy.FFmpegPlayer.VIDEO_RENDER_OPENGL;

public class VideoOpenGLActivity extends AppCompatActivity implements GLSurfaceView.Renderer, FFmpegPlayer.EventCallback {

    GLSurfaceView mGLSurfaceView;
    private FFmpegPlayer ffmpegPlayer;

    private static final int PERMISSION_REQUEST_CODE = 1;
    private static final String[] REQUEST_PERMISSIONS = {Manifest.permission.WRITE_EXTERNAL_STORAGE,};

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_video_open_glactivity);
        ffmpegPlayer = new FFmpegPlayer();
        ffmpegPlayer.addEventCallback(this);
        ffmpegPlayer.native_init(VIDEO_RENDER_OPENGL, "/data/data/com.example.ffmpegstudy/cache/testvideo1.mp4", null);

        mGLSurfaceView = findViewById(R.id.gl_surface_view);
        mGLSurfaceView.setEGLContextClientVersion(3);
        mGLSurfaceView.setRenderer(this);
//        第一种模式（RENDERMODE_CONTINUOUSLY）：
//        连续不断的刷，画完一幅图马上又画下一幅。这种模式很明显是用来画动画的；
//
//        第二种模式（RENDERMODE_WHEN_DIRTY）：
//        只有在需要重画的时候才画下一幅。这种模式就比较节约CPU和GPU一些，适合用来画不经常需要刷新的情况。
        mGLSurfaceView.setRenderMode(GLSurfaceView.RENDERMODE_WHEN_DIRTY);

    }

    @Override
    protected void onResume() {
        super.onResume();
        if (!hasPermissionsGranted(REQUEST_PERMISSIONS)) {
            ActivityCompat.requestPermissions(this, REQUEST_PERMISSIONS, PERMISSION_REQUEST_CODE);
        } else {
            if (ffmpegPlayer != null) {
                ffmpegPlayer.native_play();
            }
        }
    }

    protected boolean hasPermissionsGranted(String[] permissions) {
        for (String permission : permissions) {
            if (ActivityCompat.checkSelfPermission(this, permission) != PackageManager.PERMISSION_GRANTED) {
                return false;
            }
        }
        return true;
    }

    @Override
    public void onSurfaceCreated(GL10 gl10, EGLConfig eglConfig) {
        ffmpegPlayer.native_OnSurfaceCreated(0);
    }

    @Override
    public void onSurfaceChanged(GL10 gl10, int w, int h) {
        ffmpegPlayer.native_OnSurfaceChanged(0, w, h);
    }

    @Override
    public void onDrawFrame(GL10 gl10) {
        ffmpegPlayer.native_OnDrawFrame(0);

    }

    @Override
    public void onPlayerEvent(int msgType, float msgValue) {
        Log.d("JNI_LOG", "onPlayerEvent() called with: msgType = [" + msgType + "], msgValue = [" + msgValue + "]");
        runOnUiThread(new Runnable() {
            @Override
            public void run() {
                switch (msgType) {
                    case MSG_DECODER_INIT_ERROR:
                        break;
                    case MSG_DECODER_READY:
                        break;
                    case MSG_DECODER_DONE:
                        break;
                    case MSG_REQUEST_RENDER:
//                        Log.d("JNI_LOG", "run MSG_REQUEST_RENDER callback ");
                        mGLSurfaceView.requestRender();
                        break;
                    case MSG_DECODING_TIME:

                        break;
                    default:
                        break;
                }
            }
        });
    }
}