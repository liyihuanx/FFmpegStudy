package com.example.ffmpegstudy;

import androidx.appcompat.app.AppCompatActivity;

import android.annotation.SuppressLint;
import android.content.Intent;
import android.os.Bundle;
import android.os.Environment;
import android.view.View;

import com.example.ffmpegstudy.audiorecord.AudioRecordActivity;
import com.example.ffmpegstudy.camera.CameraRecordActivity;
import com.example.ffmpegstudy.databinding.ActivityMainBinding;
import com.example.ffmpegstudy.mediacodec.MediaCodecActivity;

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
        binding.AudioRecord.setOnClickListener(this);
        binding.MediaCodecDecode.setOnClickListener(this);
        binding.NewMediaCodecDecode.setOnClickListener(this);
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
                className = CameraRecordActivity.class;
                break;
            }
            case R.id.AudioRecord: {
                className = AudioRecordActivity.class;
                break;
            }

            case R.id.MediaCodecDecode: {
                className = MediaCodecActivity.class;
                break;
            }

            case R.id.NewMediaCodecDecode: {
                className = NewMediaCodecActivity.class;
                break;
            }

            default:
                className = ANativeActivity.class;
        }

        Intent intent = new Intent(this, className);
        startActivity(intent);
    }
}