package com.example.ffmpegstudy.camera;

import androidx.annotation.NonNull;
import androidx.appcompat.app.AppCompatActivity;
import androidx.core.app.ActivityCompat;

import android.Manifest;
import android.content.pm.PackageManager;
import android.hardware.camera2.CameraCharacteristics;
import android.opengl.GLSurfaceView;
import android.os.Bundle;
import android.os.Environment;
import android.util.Log;
import android.widget.Toast;

import com.example.ffmpegstudy.R;

import java.io.File;
import java.text.SimpleDateFormat;
import java.util.GregorianCalendar;
import java.util.Locale;

import static com.example.ffmpegstudy.camera.FFMediaRecord.RECORDER_TYPE_SINGLE_VIDEO;

public class CameraRecordActivity extends AppCompatActivity implements Camera2FrameCallback {

    private int fps = 25;
    private Camera2Help camera2Help;
    private static final int CAMERA_PERMISSION_REQUEST_CODE = 1;

    private static final String[] REQUEST_PERMISSIONS = {Manifest.permission.CAMERA, Manifest.permission.WRITE_EXTERNAL_STORAGE};

    private FFMediaRecord mMediaRecorder;

    private GLSurfaceView glCameraPreview;

    private String mOutUrl;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_camera);

        glCameraPreview = findViewById(R.id.glCameraPreview);

        camera2Help = new Camera2Help(this);
        camera2Help.setCamera2FrameCallback(this);


        mMediaRecorder = new FFMediaRecord();
        mMediaRecorder.init(glCameraPreview);

        findViewById(R.id.btnRecordVideo).setOnClickListener(v -> {
            int frameWidth = camera2Help.getPreviewSize().getWidth();
            int frameHeight = camera2Help.getPreviewSize().getHeight();
            int fps = 25;
            int bitRate = (int) (frameWidth * frameHeight * fps * 0.25);
            mOutUrl = getOutFile(".mp4").getAbsolutePath();
            mMediaRecorder.startRecord(RECORDER_TYPE_SINGLE_VIDEO, mOutUrl, frameWidth, frameHeight, bitRate, fps);

        });

        findViewById(R.id.btnStopVideo).setOnClickListener(v -> {
            Toast.makeText(CameraRecordActivity.this, "正在编码中.....", Toast.LENGTH_LONG).show();
            new Thread(() -> {
                mMediaRecorder.stopRecord();
                runOnUiThread(() -> Toast.makeText(CameraRecordActivity.this, "编码完成！", Toast.LENGTH_LONG).show());
            }).start();
        });

    }

    @Override
    protected void onResume() {
        super.onResume();
        if (hasPermissionsGranted(REQUEST_PERMISSIONS)) {
            camera2Help.startCamera();
        } else {
            ActivityCompat.requestPermissions(this, REQUEST_PERMISSIONS, CAMERA_PERMISSION_REQUEST_CODE);
        }

        updateTransformMatrix(camera2Help.getCameraId());

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
        if (requestCode == CAMERA_PERMISSION_REQUEST_CODE) {
            if (hasPermissionsGranted(REQUEST_PERMISSIONS)) {
                camera2Help.startCamera();
            } else {
                Toast.makeText(this, "We need the camera permission.", Toast.LENGTH_SHORT).show();
            }
        } else {
            super.onRequestPermissionsResult(requestCode, permissions, grantResults);
        }
    }

    @Override
    public void onPreviewFrame(byte[] data, int width, int height) {
        mMediaRecorder.onPreviewFrame(FFMediaRecord.IMAGE_FORMAT_I420, data, width, height);
        mMediaRecorder.requestRender();
    }

    @Override
    public void onCaptureFrame(byte[] data, int width, int height) {

    }


    public void updateTransformMatrix(String cameraId) {
        if (Integer.valueOf(cameraId) == CameraCharacteristics.LENS_FACING_FRONT) {
            mMediaRecorder.setTransformMatrix(90, 0);
        } else {
            mMediaRecorder.setTransformMatrix(90, 1);
        }
    }

    private static final String RESULT_IMG_DIR = "Atestvideo";
    private static final SimpleDateFormat DateTime_FORMAT = new SimpleDateFormat("yyyyMMddHHmmss", Locale.US);

    public static final File getOutFile(final String ext) {
        final File dir = new File(Environment.getExternalStorageDirectory(), RESULT_IMG_DIR);
        Log.d("JNI_LOG", "path=" + dir.toString());
        dir.mkdirs();
        if (dir.canWrite()) {
            return new File(dir, "video_" + getDateTimeString() + ext);
        }
        return null;
    }

    private static final String getDateTimeString() {
        final GregorianCalendar now = new GregorianCalendar();
        return DateTime_FORMAT.format(now.getTime());
    }
}