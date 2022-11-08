package com.example.ffmpegstudy.camera;

import androidx.annotation.NonNull;
import androidx.appcompat.app.AppCompatActivity;
import androidx.core.app.ActivityCompat;

import android.Manifest;
import android.content.pm.PackageManager;
import android.graphics.SurfaceTexture;
import android.hardware.camera2.CameraCharacteristics;
import android.opengl.GLSurfaceView;
import android.os.Bundle;
import android.view.TextureView;
import android.view.View;
import android.widget.Toast;

import com.example.ffmpegstudy.R;

public class CameraActivity extends AppCompatActivity implements Camera2FrameCallback {

    private int fps = 25;
    private Camera2Help camera2Help;
    private static final int CAMERA_PERMISSION_REQUEST_CODE = 1;

    private static final String[] REQUEST_PERMISSIONS = {Manifest.permission.CAMERA, Manifest.permission.WRITE_EXTERNAL_STORAGE};

    private FFMediaRecord mMediaRecorder;

    private GLSurfaceView glCameraPreview;
    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_camera);

        glCameraPreview = findViewById(R.id.glCameraPreview);

        camera2Help = new Camera2Help(this);
        camera2Help.setCamera2FrameCallback(this);


        mMediaRecorder = new FFMediaRecord();
        mMediaRecorder.init(glCameraPreview);
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
}