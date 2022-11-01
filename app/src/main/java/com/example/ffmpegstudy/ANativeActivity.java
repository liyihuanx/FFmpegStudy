package com.example.ffmpegstudy;

import static com.example.ffmpegstudy.FFmpegPlayer.VIDEO_RENDER_ANWINDOW;

import androidx.annotation.NonNull;
import androidx.appcompat.app.AppCompatActivity;

import android.os.Bundle;
import android.util.Log;
import android.view.SurfaceHolder;
import android.view.SurfaceView;
import android.view.View;

public class ANativeActivity extends AppCompatActivity implements SurfaceHolder.Callback {

    private SurfaceHolder holder;
    private FFmpegPlayer ffmpegPlayer;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_anative);

        SurfaceView surfaceView = findViewById(R.id.surface_view);
        holder = surfaceView.getHolder();
        holder.addCallback(this);
    }


    @Override
    public void surfaceCreated(@NonNull SurfaceHolder surfaceHolder) {
        Log.d("JNI_LOG", "surfaceCreated");
        ffmpegPlayer = new FFmpegPlayer();
        ffmpegPlayer.native_init(VIDEO_RENDER_ANWINDOW, "/data/data/com.example.ffmpegstudy/cache/testvideo1.mp4", surfaceHolder.getSurface());
    }

    @Override
    public void surfaceChanged(@NonNull SurfaceHolder surfaceHolder, int format, int w, int h) {
        Log.d("JNI_LOG", "surfaceChanged");
        ffmpegPlayer.native_play();
    }

    @Override
    public void surfaceDestroyed(@NonNull SurfaceHolder surfaceHolder) {

    }
}