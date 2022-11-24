package com.example.ffmpegstudy.mediacodec;

import androidx.appcompat.app.AppCompatActivity;

import android.graphics.Bitmap;
import android.os.Bundle;
import android.view.View;

import com.example.ffmpegstudy.R;

public class NewMediaCodecActivity extends AppCompatActivity {

    private VideoDecoder videoDecoder;
    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_new_media_codec);
        videoDecoder = new VideoDecoder();
        videoDecoder.setPreviewCallback(new VideoDecoder.PreviewCallback() {
            @Override
            public void info(int width, int height, int fps) {

            }

            @Override
            public void getBitmap(Bitmap bitmap) {

            }

            @Override
            public void progress(int progress) {

            }
        });
        findViewById(R.id.btnDecoderStart).setOnClickListener(v -> {
            videoDecoder.startDecode();
        });
    }
}