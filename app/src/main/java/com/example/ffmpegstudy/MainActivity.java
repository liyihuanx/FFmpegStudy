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

import com.example.ffmpegstudy.camera.CameraActivity;
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
        binding.CameraRender.setOnClickListener(this);

    }


    @SuppressLint("NonConstantResourceId")
    @Override
    public void onClick(View v) {
        Class<?> className;
        switch (v.getId()) {
            case R.id.ANativeRender: {
                className = ANativeActivity.class;
            }
            break;

            case R.id.OpenGLRender: {
                className = VideoOpenGLActivity.class;
                break;
            }
            case R.id.CameraRender: {
                className = CameraActivity.class;
                break;
            }
            default:
                className = ANativeActivity.class;
        }

        Intent intent = new Intent(this, className);
        startActivity(intent);
    }
}