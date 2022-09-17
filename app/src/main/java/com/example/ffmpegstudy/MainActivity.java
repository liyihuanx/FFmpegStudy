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

public class MainActivity extends AppCompatActivity implements SurfaceHolder.Callback {

    // Used to load the 'ffmpegstudy' library on application startup.
    static {
        System.loadLibrary("ffmpegstudy");
    }

    private ActivityMainBinding binding;

    private SurfaceHolder holder;

    // /storage/emulated/0/Atestvideo/videotest1.mp4

    private String videoUrl = Environment.getExternalStorageDirectory().getPath() + "/Atestvideo/videotest1.mp4";

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);

        binding = ActivityMainBinding.inflate(getLayoutInflater());
        setContentView(binding.getRoot());

        binding.helloFFmpeg.setOnClickListener(v -> {
            String str = native_helloFFmpeg();
            native_Mp4toYuv("/data/data/com.example.ffmpegstudy/cache/testvideo1.mp4", "/data/data/com.example.ffmpegstudy/cache/testvideo1.yuv");

        });

        holder = binding.sfView.getHolder();
        holder.addCallback(this);

    }

    @Override
    public void surfaceCreated(@NonNull SurfaceHolder surfaceHolder) {
        new Thread(new Runnable() {
            @Override
            public void run() {
//                native_play(surfaceHolder.getSurface());
                native_playVideo("/data/data/com.example.ffmpegstudy/cache/testvideo1.mp4", surfaceHolder.getSurface());
            }
        }).start();
    }

    @Override
    public void surfaceChanged(@NonNull SurfaceHolder surfaceHolder, int i, int i1, int i2) {

    }

    @Override
    public void surfaceDestroyed(@NonNull SurfaceHolder surfaceHolder) {

    }


    /**
     * A native method that is implemented by the 'ffmpegstudy' native library,
     * which is packaged with this application.
     */
    public native String native_helloFFmpeg();

    public native void native_playVideo(String url, Surface surfaceView);
    public native int native_Mp4toYuv(String input, String output);

    public native int native_play(Surface surfaceView);
}