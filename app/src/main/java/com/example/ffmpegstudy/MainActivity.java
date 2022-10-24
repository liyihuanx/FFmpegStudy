package com.example.ffmpegstudy;

import androidx.annotation.NonNull;
import androidx.appcompat.app.AppCompatActivity;

import android.os.Bundle;
import android.os.Environment;
import android.util.Log;
import android.view.Surface;
import android.view.SurfaceHolder;
import android.view.SurfaceView;
import android.widget.TextView;

import com.example.ffmpegstudy.databinding.ActivityMainBinding;

import static com.example.ffmpegstudy.FFmpegPlayer.VIDEO_RENDER_ANWINDOW;

public class MainActivity extends AppCompatActivity implements SurfaceHolder.Callback {


    private ActivityMainBinding binding;

    private SurfaceHolder holder;

    // /storage/emulated/0/Atestvideo/videotest1.mp4

    private String videoUrl = Environment.getExternalStorageDirectory().getPath() + "/Atestvideo/videotest1.mp4";

    private FFmpegPlayer ffmpegPlayer;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);

        binding = ActivityMainBinding.inflate(getLayoutInflater());
        setContentView(binding.getRoot());

        holder = binding.sfView.getHolder();
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