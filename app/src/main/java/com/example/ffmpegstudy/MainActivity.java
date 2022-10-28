package com.example.ffmpegstudy;

import androidx.annotation.NonNull;
import androidx.appcompat.app.AppCompatActivity;

import android.annotation.SuppressLint;
import android.content.Intent;
import android.os.Bundle;
import android.os.Environment;
import android.util.Log;
import android.view.Surface;
import android.view.SurfaceHolder;
import android.view.SurfaceView;
import android.view.View;
import android.widget.TextView;

import com.example.ffmpegstudy.databinding.ActivityMainBinding;

import static com.example.ffmpegstudy.FFmpegPlayer.VIDEO_RENDER_ANWINDOW;

public class MainActivity extends AppCompatActivity implements View.OnClickListener {


    private ActivityMainBinding binding;

    // /storage/emulated/0/Atestvideo/videotest1.mp4

    private String videoUrl = Environment.getExternalStorageDirectory().getPath() + "/Atestvideo/videotest1.mp4";

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);

        binding = ActivityMainBinding.inflate(getLayoutInflater());
        setContentView(binding.getRoot());


        binding.ANativeRender.setOnClickListener(this);
        binding.OpenGLRender.setOnClickListener(this);


    }


    @SuppressLint("NonConstantResourceId")
    @Override
    public void onClick(View v) {
        switch (v.getId()) {
            case R.id.ANativeRender: {
                Intent intent = new Intent(this, ANativeActivity.class);
                startActivity(intent);
            }
            break;

            case R.id.OpenGLRender: {
                Intent intent = new Intent(this, VideoOpenGLActivity.class);
                startActivity(intent);
            }
            break;
        }
    }
}