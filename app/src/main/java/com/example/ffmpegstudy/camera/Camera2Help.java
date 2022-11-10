package com.example.ffmpegstudy.camera;

import android.Manifest;
import android.app.assist.AssistStructure;
import android.content.Context;
import android.content.pm.PackageManager;
import android.graphics.ImageFormat;
import android.graphics.SurfaceTexture;
import android.hardware.camera2.CameraAccessException;
import android.hardware.camera2.CameraCaptureSession;
import android.hardware.camera2.CameraCharacteristics;
import android.hardware.camera2.CameraDevice;
import android.hardware.camera2.CameraManager;
import android.hardware.camera2.CaptureRequest;
import android.hardware.camera2.TotalCaptureResult;
import android.hardware.camera2.params.StreamConfigurationMap;
import android.media.Image;
import android.media.ImageReader;
import android.os.Handler;
import android.os.HandlerThread;
import android.util.AndroidRuntimeException;
import android.util.Log;
import android.util.Size;
import android.view.Surface;

import java.util.Arrays;
import java.util.List;
import java.util.concurrent.Semaphore;
import java.util.concurrent.TimeUnit;

import androidx.annotation.NonNull;
import androidx.core.content.ContextCompat;

/**
 * @ClassName: CameraHelp
 * @Description: java类作用描述
 * @Author: liyihuan
 * @Date: 2022/11/1 20:49
 */
class Camera2Help {
    private static final String TAG = "Camera2Help";
    private static final int DEFAULT_CAMERA_ID = 0;
    private final float THRESHOLD = 0.001f;

    private Context mContext;
    private CameraManager mCameraManager;

    // 相机会话
    private CameraCaptureSession mCameraCaptureSession;
    // 拍照的Request
    private CaptureRequest mPreviewRequest;
    // 相机设备
    private CameraDevice mCameraDevice;
    private String mCameraId;
    // 支持的摄像头集合
    private String[] mSupportCameraIds;
    // 预览ImageReader
    private ImageReader mPreviewImageReader;
    // 拍照
    private ImageReader mCaptureImageReader;
    // 角度
    private Integer mSensorOrientation;

    private Surface mPreviewSurface;

    private Semaphore mCameraLock = new Semaphore(1);

    // 默认的预览，拍照宽高
    private Size mDefaultPreviewSize = new Size(1280, 720);
    private Size mDefaultCaptureSize = new Size(1280, 720);

    // 支持的
    private List<Size> mSupportPreviewSize;
    private List<Size> mSupportPictureSize;

    // 实际上选择的
    private Size mPreviewSize;
    private Size mPictureSize;

    // 拿到数据帧之后的回调
    private Camera2FrameCallback mCamera2FrameCallback;
    private Handler mBackgroundHandler;
    private HandlerThread mBackgroundThread;

    // 相机状态回调
    private final CameraDevice.StateCallback mStateCallback = new CameraDevice.StateCallback() {

        @Override
        public void onOpened(@NonNull CameraDevice cameraDevice) {
            Log.d(TAG, "Camera onOpened success");
            // This method is called when the camera is opened.  We start camera preview here.
            mCameraLock.release();
            mCameraDevice = cameraDevice;
            // 打开相机后，创建会话
            createCaptureSession();
        }

        @Override
        public void onDisconnected(@NonNull CameraDevice cameraDevice) {
            Log.d(TAG, "Camera onDisconnected");
            mCameraLock.release();
            cameraDevice.close();
            mCameraDevice = null;
        }

        @Override
        public void onError(@NonNull CameraDevice cameraDevice, int error) {
            Log.d(TAG, "Camera onError: " + error);
            mCameraLock.release();
            cameraDevice.close();
            mCameraDevice = null;
        }
    };

    public void setCamera2FrameCallback(Camera2FrameCallback mCamera2FrameCallback) {
        this.mCamera2FrameCallback = mCamera2FrameCallback;
    }

    // 数据流回调
    private ImageReader.OnImageAvailableListener mOnPreviewImageAvailableListener = new ImageReader.OnImageAvailableListener() {
        @Override
        public void onImageAvailable(ImageReader reader) {
            Image image = reader.acquireLatestImage();
            if (image != null) {
                if (mCamera2FrameCallback != null) {
                    mCamera2FrameCallback.onPreviewFrame(CameraUtil.YUV_420_888_data(image), image.getWidth(), image.getHeight());
                }
                image.close();
            }
        }
    };

    private ImageReader.OnImageAvailableListener mOnCaptureImageAvailableListener = new ImageReader.OnImageAvailableListener() {
        @Override
        public void onImageAvailable(ImageReader reader) {
            Image image = reader.acquireLatestImage();
            if (image != null) {
                if (mCamera2FrameCallback != null) {
                    mCamera2FrameCallback.onCaptureFrame(CameraUtil.YUV_420_888_data(image), image.getWidth(), image.getHeight());
                }
                image.close();
            }
        }
    };

    public Camera2Help(Context context) {
        this.mContext = context;
        initCamera2();
    }


    private void initCamera2() {
        mCameraManager = (CameraManager) mContext.getSystemService(Context.CAMERA_SERVICE);

        try {
            // 支持的摄像头列表
            mSupportCameraIds = mCameraManager.getCameraIdList();
            // 是否支持后置摄像头
            if (!checkCameraIdSupport(String.valueOf(DEFAULT_CAMERA_ID))) {
                throw new AndroidRuntimeException("Don't support the camera id: " + DEFAULT_CAMERA_ID);
            }

            mCameraId = String.valueOf(DEFAULT_CAMERA_ID);
            // 获取相机信息
            getCameraInfo(mCameraId);

        } catch (CameraAccessException e) {
            e.printStackTrace();
        }
    }


    private boolean checkCameraIdSupport(String cameraId) {
        boolean isSupported = false;
        for (String id : mSupportCameraIds) {
            if (cameraId.equals(id)) {
                isSupported = true;
            }
        }
        return isSupported;
    }

    /**
     * 获取相机信息
     *
     * @param cameraId
     */
    private void getCameraInfo(String cameraId) {
        CameraCharacteristics characteristics = null;
        try {
            characteristics = mCameraManager.getCameraCharacteristics(cameraId);
        } catch (CameraAccessException e) {
            e.printStackTrace();
        }

        // 相机设备支持的可用流的配置，包括最小帧间隔、不同格式、大小组合的失帧时长
        StreamConfigurationMap streamConfigs = characteristics.get(CameraCharacteristics.SCALER_STREAM_CONFIGURATION_MAP);

        if (streamConfigs != null) {
            // 相机支持的所有分辨率，下一步就是获取最合适的分辨率
            mSupportPreviewSize = Arrays.asList(streamConfigs.getOutputSizes(SurfaceTexture.class));

            boolean supportDefaultSize = false;
            Size sameRatioSize = null;
            // 宽高比
            float defaultRatio = mDefaultPreviewSize.getWidth() * 1.0f / mDefaultPreviewSize.getHeight();
            // 预览的大小取中间值
            mPreviewSize = mSupportPreviewSize.get(mSupportPreviewSize.size() / 2);
            Log.d(TAG, "mPreviewSize width: " + mPreviewSize.getWidth() + ",height:" + mPreviewSize.getHeight());

            for (Size size : mSupportPreviewSize) {
                Log.d(TAG, "mSupportPreviewSize " + size.getWidth() + "x" + size.getHeight());
                // 每个分辨率的宽高比
                float ratio = size.getWidth() * 1.0f / size.getHeight();
                // 宽高比在误差在预期内的，做记录
                if (Math.abs(ratio - defaultRatio) < THRESHOLD) {
                    Log.d(TAG, "mSupportPreviewSize sameRatioSize == size" + size.getWidth() + "x" + size.getHeight());
                    sameRatioSize = size;
                }
                // 最合适的分辨率
                if (mDefaultPreviewSize.getWidth() == size.getWidth() && mDefaultPreviewSize.getHeight() == size.getHeight()) {
                    Log.d(TAG, "initCamera2Wrapper() called supportDefaultSize ");
                    supportDefaultSize = true;
                    break;
                }
            }

            // 赋值
            if (supportDefaultSize) {
                mPreviewSize = mDefaultPreviewSize;
            } else if (sameRatioSize != null) {
                mPreviewSize = sameRatioSize;
            }
            //Log.d(TAG, "initCamera2Wrapper() called mPreviewSize[w,h] = [" + mPreviewSize.getWidth() + "," + mPictureSize.getHeight() + "]");


            // 拍照的最佳分辨率
            supportDefaultSize = false;
            sameRatioSize = null;
            defaultRatio = mDefaultCaptureSize.getWidth() * 1.0f / mDefaultCaptureSize.getHeight();
            mSupportPictureSize = Arrays.asList(streamConfigs.getOutputSizes(ImageFormat.YUV_420_888));
            mPictureSize = mSupportPictureSize.get(0);
            for (Size size : mSupportPictureSize) {
                Log.d(TAG, "initCamera2Wrapper() called mSupportPictureSize " + size.getWidth() + "x" + size.getHeight());
                float ratio = size.getWidth() * 1.0f / size.getHeight();
                if (Math.abs(ratio - defaultRatio) < THRESHOLD) {
                    sameRatioSize = size;
                }

                if (mDefaultCaptureSize.getWidth() == size.getWidth() && mDefaultCaptureSize.getHeight() == size.getHeight()) {
                    supportDefaultSize = true;
                    break;
                }
            }
            if (supportDefaultSize) {
                mPictureSize = mDefaultCaptureSize;
            } else if (sameRatioSize != null) {
                mPictureSize = sameRatioSize;
            }
        }
        mSensorOrientation = characteristics.get(CameraCharacteristics.SENSOR_ORIENTATION);
        Log.d(TAG, "initCamera2Wrapper() called mSensorOrientation = " + mSensorOrientation);

    }


    private CameraCaptureSession.StateCallback mSessionStateCallback = new CameraCaptureSession.StateCallback() {
        @Override
        public void onConfigured(@NonNull CameraCaptureSession session) {
            mCameraCaptureSession = session;
            try {
                mPreviewRequest = createPreviewRequest();
                if (mPreviewRequest != null) {
                    // 预览本质上是不断重复执行的 Capture 操作，每一次 Capture 都会把预览画面输出到对应的 Surface 上，
                    session.setRepeatingRequest(mPreviewRequest, null, mBackgroundHandler);
                } else {
                    Log.e(TAG, "captureRequest is null");
                }
            } catch (CameraAccessException e) {
                Log.e(TAG, "onConfigured " + e.toString());
            }
        }

        @Override
        public void onConfigureFailed(@NonNull CameraCaptureSession session) {
            Log.e(TAG, "onConfigureFailed");
        }
    };

    /**
     * 创建一个相机会话
     */
    private void createCaptureSession() {
        try {
            if (null == mCameraDevice || null == mPreviewSurface || null == mCaptureImageReader)
                return;
            mCameraDevice.createCaptureSession(Arrays.asList(mPreviewSurface, mCaptureImageReader.getSurface()), mSessionStateCallback, mBackgroundHandler);

        } catch (CameraAccessException e) {
            Log.e(TAG, "createCaptureSession " + e.toString());
        }
    }

    public void startCamera(SurfaceTexture previewSurfaceTex) {
        initImageRender(previewSurfaceTex);
        openCamera();
    }

    public void startCamera() {
        initImageRender(null);
        openCamera();
    }

    public String getCameraId() {
        return mCameraId;
    }

    /**
     * 打开相机
     */
    public void openCamera() {

        if (ContextCompat.checkSelfPermission(mContext, Manifest.permission.CAMERA) != PackageManager.PERMISSION_GRANTED) {
            return;
        }

        CameraManager manager = (CameraManager) mContext.getSystemService(Context.CAMERA_SERVICE);
        try {
            if (!mCameraLock.tryAcquire(2500, TimeUnit.MILLISECONDS)) {
                throw new RuntimeException("Time out waiting to lock camera opening.");
            }
            manager.openCamera(mCameraId, mStateCallback, mBackgroundHandler);
        } catch (CameraAccessException e) {
            Log.e(TAG, "Cannot access the camera." + e.toString());
        } catch (InterruptedException e) {
            throw new RuntimeException("Interrupted while trying to lock camera opening.", e);
        }
    }

    /**
     * 创建ImageReader实例设置一些属性参数
     */
    private void initImageRender(SurfaceTexture previewSurfaceTex) {
        startBackgroundThread();
        // 预览
        if (previewSurfaceTex != null) {
            previewSurfaceTex.setDefaultBufferSize(mPreviewSize.getWidth(), mPreviewSize.getHeight());
            mPreviewSurface = new Surface(previewSurfaceTex);
        } else if (mPreviewImageReader == null && mPreviewSize != null) {
            // 实例化
            mPreviewImageReader = ImageReader.newInstance(mPreviewSize.getWidth(), mPreviewSize.getHeight(), ImageFormat.YUV_420_888, 2);
            // 设置监听当图片流可用的时候的监听器,即为拍照，预览之后产生的流，给到底层OpenGL渲染
            mPreviewImageReader.setOnImageAvailableListener(mOnPreviewImageAvailableListener, mBackgroundHandler);

            mPreviewSurface = mPreviewImageReader.getSurface();
        }

        // 拍照
        if (mCaptureImageReader == null && mPictureSize != null) {
            mCaptureImageReader = ImageReader.newInstance(mPictureSize.getWidth(), mPictureSize.getHeight(), ImageFormat.YUV_420_888, 2);
            mCaptureImageReader.setOnImageAvailableListener(mOnCaptureImageAvailableListener, mBackgroundHandler);
        }
    }

    private void startBackgroundThread() {
        mBackgroundThread = new HandlerThread("Camera2Background");
        mBackgroundThread.start();
        mBackgroundHandler = new Handler(mBackgroundThread.getLooper());
    }

    private void stopBackgroundThread() {
        if (mBackgroundThread != null) {
            mBackgroundThread.quitSafely();
            try {
                mBackgroundThread.join();
                mBackgroundThread = null;
                mBackgroundHandler = null;
            } catch (InterruptedException e) {
                e.printStackTrace();
            }
        }
    }

    private void closeCamera() {
        Log.d(TAG, "closeCamera() called");
        try {
            mCameraLock.acquire();
            if (null != mCameraCaptureSession) {
                mCameraCaptureSession.close();
                mCameraCaptureSession = null;
            }
            if (null != mCameraDevice) {
                mCameraDevice.close();
                mCameraDevice = null;
            }
            if (null != mPreviewImageReader) {
                mPreviewImageReader.close();
                mPreviewImageReader = null;
            }

            if (null != mCaptureImageReader) {
                mCaptureImageReader.close();
                mCaptureImageReader = null;
            }
        } catch (InterruptedException e) {
            throw new RuntimeException("Interrupted while trying to lock camera closing.", e);
        } finally {
            mCameraLock.release();
        }

    }

    private CaptureRequest createPreviewRequest() {
        if (null == mCameraDevice || mPreviewSurface == null) return null;
        try {
            CaptureRequest.Builder builder = mCameraDevice.createCaptureRequest(CameraDevice.TEMPLATE_PREVIEW);
            builder.addTarget(mPreviewSurface);
            return builder.build();
        } catch (CameraAccessException e) {
            Log.e(TAG, e.getMessage());
            return null;
        }
    }

    public void capture() {
        if (mCameraDevice == null) return;
        final CaptureRequest.Builder captureBuilder;
        try {
            captureBuilder = mCameraDevice.createCaptureRequest(CameraDevice.TEMPLATE_STILL_CAPTURE);
            captureBuilder.addTarget(mCaptureImageReader.getSurface());

            // Use the same AE and AF modes as the preview.
            captureBuilder.set(CaptureRequest.CONTROL_AF_MODE, CaptureRequest.CONTROL_AF_MODE_CONTINUOUS_PICTURE);

            // Orientation
            CameraCaptureSession.CaptureCallback CaptureCallback = new CameraCaptureSession.CaptureCallback() {

                @Override
                public void onCaptureCompleted(@NonNull CameraCaptureSession session, @NonNull CaptureRequest request, @NonNull TotalCaptureResult result) {
                    if (mPreviewRequest != null && mCameraCaptureSession != null) {
                        try {
                            mCameraCaptureSession.setRepeatingRequest(mPreviewRequest, null, mBackgroundHandler);
                        } catch (CameraAccessException e) {
                            e.printStackTrace();
                        }
                    }
                }

            };

            mCameraCaptureSession.stopRepeating();
            mCameraCaptureSession.abortCaptures();
            mCameraCaptureSession.capture(captureBuilder.build(), CaptureCallback, null);
        } catch (CameraAccessException e) {
            e.printStackTrace();
        }
    }

    public Size getPreviewSize() {
        return mPreviewSize;
    }

}
