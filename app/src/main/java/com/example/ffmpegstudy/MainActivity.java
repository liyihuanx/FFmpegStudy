package com.example.ffmpegstudy;

import androidx.appcompat.app.AppCompatActivity;

import android.os.Bundle;
import android.os.Environment;
import android.util.Log;
import android.widget.TextView;

import com.example.ffmpegstudy.databinding.ActivityMainBinding;

public class MainActivity extends AppCompatActivity {

    // Used to load the 'ffmpegstudy' library on application startup.
    static {
        System.loadLibrary("ffmpegstudy");
    }

    private ActivityMainBinding binding;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);

        binding = ActivityMainBinding.inflate(getLayoutInflater());
        setContentView(binding.getRoot());

        binding.helloFFmpeg.setOnClickListener(v -> {
            String str = native_helloFFmpeg();
            Log.d("QWER", "onCreate: " + str);
        });

    }

    /**
     * A native method that is implemented by the 'ffmpegstudy' native library,
     * which is packaged with this application.
     */
    public native String native_helloFFmpeg();



}