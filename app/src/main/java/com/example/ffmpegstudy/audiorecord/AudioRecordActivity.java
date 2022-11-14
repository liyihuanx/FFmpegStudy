package com.example.ffmpegstudy.audiorecord;

import androidx.annotation.NonNull;
import androidx.appcompat.app.AppCompatActivity;
import androidx.core.app.ActivityCompat;

import android.Manifest;
import android.content.pm.PackageManager;
import android.os.Bundle;
import android.os.Environment;
import android.util.Log;
import android.widget.Toast;

import com.example.ffmpegstudy.R;
import com.example.ffmpegstudy.camera.CameraRecordActivity;
import com.example.ffmpegstudy.camera.FFMediaRecord;

import java.io.File;
import java.text.SimpleDateFormat;
import java.util.GregorianCalendar;
import java.util.Locale;

import static com.example.ffmpegstudy.camera.FFMediaRecord.RECORDER_TYPE_SINGLE_AUDIO;

public class AudioRecordActivity extends AppCompatActivity implements AudioRecorder.AudioRecorderCallback {

    private FFMediaRecord mMediaRecorder;
    private AudioRecorder mAudioRecorder;

    private static final int AUDIO_PERMISSION_REQUEST_CODE = 1;
    private static final String[] REQUEST_PERMISSIONS = {Manifest.permission.RECORD_AUDIO, Manifest.permission.WRITE_EXTERNAL_STORAGE,};
    private String mOutUrl;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_audio_record);


        mMediaRecorder = new FFMediaRecord();
        mMediaRecorder.init(null);

        findViewById(R.id.btnStartAudio).setOnClickListener(v -> {
            mOutUrl = getOutFile(".aac").getAbsolutePath();
            mMediaRecorder.startRecord(RECORDER_TYPE_SINGLE_AUDIO, mOutUrl, 0, 0, 0, 0);
            mAudioRecorder = new AudioRecorder( this);
            mAudioRecorder.start();
            Toast.makeText(this, "开始录音.....", Toast.LENGTH_LONG).show();
        });

        findViewById(R.id.btnStopAudio).setOnClickListener(v -> {
            mAudioRecorder.interrupt();
            try {
                mAudioRecorder.join();
            } catch (InterruptedException e) {
                e.printStackTrace();
            }
            mAudioRecorder = null;

            Toast.makeText(this, "正在编码中.....", Toast.LENGTH_LONG).show();
            new Thread(() -> {
                mMediaRecorder.stopRecord();
                runOnUiThread(() -> Toast.makeText(this, "编码完成！", Toast.LENGTH_LONG).show());
            }).start();

        });
    }

    @Override
    protected void onResume() {
        super.onResume();
        if (!hasPermissionsGranted(REQUEST_PERMISSIONS)) {
            ActivityCompat.requestPermissions(this, REQUEST_PERMISSIONS, AUDIO_PERMISSION_REQUEST_CODE);
        }
    }

    @Override
    protected void onDestroy() {
        super.onDestroy();
        mMediaRecorder.unInit();
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
    public void onRequestPermissionsResult(int requestCode, @NonNull String[] permissions, @NonNull int[] grantResults) {
        if (requestCode == AUDIO_PERMISSION_REQUEST_CODE) {
            if (!hasPermissionsGranted(REQUEST_PERMISSIONS)) {
                Toast.makeText(this, "We need the camera permission.", Toast.LENGTH_SHORT).show();
            }
        } else {
            super.onRequestPermissionsResult(requestCode, permissions, grantResults);
        }
    }


    private static final String RESULT_IMG_DIR = "Atestvideo";
    private static final SimpleDateFormat DateTime_FORMAT = new SimpleDateFormat("yyyyMMddHHmmss", Locale.US);

    public static final File getOutFile(final String ext) {
        final File dir = new File(Environment.getExternalStorageDirectory(), RESULT_IMG_DIR);
        Log.d("JNI_LOG", "path=" + dir.toString());
        dir.mkdirs();
        if (dir.canWrite()) {
            return new File(dir, "audio_" + getDateTimeString() + ext);
        }
        return null;
    }

    private static final String getDateTimeString() {
        final GregorianCalendar now = new GregorianCalendar();
        return DateTime_FORMAT.format(now.getTime());
    }

    @Override
    public void onAudioData(byte[] data, int dataSize) {
        mMediaRecorder.onAudioData(data, dataSize);
    }

    @Override
    public void onError(String msg) {

    }
}