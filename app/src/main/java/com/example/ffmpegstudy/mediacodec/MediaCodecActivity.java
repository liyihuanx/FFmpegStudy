package com.example.ffmpegstudy.mediacodec;

import androidx.annotation.NonNull;
import androidx.appcompat.app.AppCompatActivity;
import androidx.constraintlayout.widget.ConstraintLayout;

import android.content.pm.ActivityInfo;
import android.graphics.Bitmap;
import android.graphics.BitmapFactory;
import android.graphics.ImageFormat;
import android.graphics.Rect;
import android.graphics.YuvImage;
import android.media.MediaCodec;
import android.media.MediaCodecInfo;
import android.media.MediaCodecList;
import android.media.MediaExtractor;
import android.media.MediaFormat;
import android.net.Uri;
import android.os.Bundle;
import android.os.Environment;
import android.util.Log;
import android.view.SurfaceHolder;
import android.view.SurfaceView;
import android.widget.LinearLayout;

import com.example.ffmpegstudy.R;

import java.io.BufferedOutputStream;
import java.io.ByteArrayOutputStream;
import java.io.File;
import java.io.FileOutputStream;
import java.io.IOException;
import java.nio.ByteBuffer;

public class MediaCodecActivity extends AppCompatActivity implements SurfaceHolder.Callback {

    private static final String TAG = "MediaCodecActivity";

    private SurfaceView mediacodecView;
//    private SurfaceHolder mediacodecHolder;

    private MediaCodec mediaCodec;
    private MediaExtractor mediaExtractor;
    private String videoUrl = "/data/data/com.example.ffmpegstudy/cache/testvideo1.mp4";

    private int videoWidth;
    private int videoHeight;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_media_codec);

        mediacodecView = findViewById(R.id.mediacodecView);
        mediacodecView.getHolder().addCallback(this);

    }

    private void initMedia() {
        mediaExtractor = new MediaExtractor();
        try {
            mediaExtractor.setDataSource(videoUrl);
            for (int i = 0; i < mediaExtractor.getTrackCount(); i++) {
                MediaFormat trackFormat = mediaExtractor.getTrackFormat(i);
                String mineType = trackFormat.getString(MediaFormat.KEY_MIME);
                if (mineType.startsWith("video/")) {
                    mediaExtractor.selectTrack(i);
                    initVideo(trackFormat);
                    break;
                }
            }
        } catch (IOException e) {
            e.printStackTrace();
        }
    }

    private void initVideo(MediaFormat mediaFormat) throws IOException {
        // 获取高度
        videoWidth = mediaFormat.getInteger(MediaFormat.KEY_WIDTH);
        // 获取高度
        videoHeight = mediaFormat.getInteger(MediaFormat.KEY_HEIGHT);

        String mineType = mediaFormat.getString(MediaFormat.KEY_MIME);
        MediaCodecInfo codecInfo = getCodecInfo(mineType);

        if (codecInfo != null) {
            // 根据视频编码创建解码器，这里是解码AVC编码的视频
            mediaCodec = MediaCodec.createDecoderByType(mineType);

            mediaCodec.configure(mediaFormat, mediacodecView.getHolder().getSurface(), null, 0);
            mediaCodec.start();

        } else {
            Log.d(TAG, "initVideo error codecInfo == null");
        }
    }

    /**
     * 获取支持的解码格式
     *
     * @param mineType
     * @return
     */
    private MediaCodecInfo getCodecInfo(String mineType) {
        MediaCodecList mediaCodecList = new MediaCodecList(MediaCodecList.ALL_CODECS);
        MediaCodecInfo[] codecInfos = mediaCodecList.getCodecInfos();
        for (MediaCodecInfo info : codecInfos) {
            if (info.isEncoder()) {
                continue;
            }

            String[] supportedTypes = info.getSupportedTypes();
            for (int i = 0; i < supportedTypes.length; i++) {
                String supportedType = supportedTypes[i];
                if (supportedType.equalsIgnoreCase(mineType)) {
                    return info;
                }
            }
        }
        return null;
    }

    private Thread decodeThread;
    private volatile boolean isDecoding = false;

    private Runnable decodeVideoRunnable = () -> {
        try {
            // 存放目标文件的数据buffer
            ByteBuffer byteBuffer = null;
            // 解码后的数据
            MediaCodec.BufferInfo outputInfo = new MediaCodec.BufferInfo();

            boolean first = false;
            long startWhen = 0;
            boolean isInput = true;
            int frameIndex = 0;

            while (isDecoding) {
                if (isInput) {
                    // 拿到可用的ByteBuffer的index
                    int index = mediaCodec.dequeueInputBuffer(10000);
                    if (index >= 0) {
                        // 取出该Buffer
                        ByteBuffer inputBuffer = mediaCodec.getInputBuffer(index);
                        // 从媒体文件中读取的一个sample数据大小
                        int sampleSize = mediaExtractor.readSampleData(inputBuffer, 0);
                        if (mediaExtractor.advance() && sampleSize > 0) {
                            // 把inputBufIndex对应的数据传入MediaCodec, getSampleTime得到当前数据的播放时间点
                            mediaCodec.queueInputBuffer(index, 0, sampleSize, mediaExtractor.getSampleTime(), 0);
                        } else {
                            // BUFFER_FLAG_END_OF_STREAM 最后一组数据
                            mediaCodec.queueInputBuffer(index, 0, 0, 0, MediaCodec.BUFFER_FLAG_END_OF_STREAM);
                            isInput = false;
                        }
                    } else {
                        continue;
                    }
                }
                // 解码
                int outputIndex = mediaCodec.dequeueOutputBuffer(outputInfo, 10000);
                ByteBuffer outputBuffer;
                if (outputIndex == MediaCodec.INFO_TRY_AGAIN_LATER) {
                    // 表示等了TIMEOUT_USEC时间长，也暂时还没有解码出成功的数据。
                    // 一般来说，一个是等待时间还不够，另一个就是输入端是B帧，需要后面一帧P帧来作为参考帧才可以解
                    Log.d(TAG, "no output from decoder available");
                } else if (outputIndex == MediaCodec.INFO_OUTPUT_BUFFERS_CHANGED) {
                    // not important for us, since we're using Surface
                    Log.d(TAG, "decoder output buffers changed");
                } else if (outputIndex == MediaCodec.INFO_OUTPUT_FORMAT_CHANGED) {
                    // 输出数据的MediaFormat发生了变化
                    MediaFormat newFormat = mediaCodec.getOutputFormat();
                    Log.d(TAG, "decoder output format changed: " + newFormat);
                } else if (outputIndex < 0) {
                    throw new RuntimeException("unexpected result from decoder.dequeueOutputBuffer: " + outputIndex);
                } else {
                    if (!first) {
                        startWhen = System.currentTimeMillis();
                        first = true;
                    }
                    try {
                        long sleepTime = outputInfo.presentationTimeUs / 1000 - (System.currentTimeMillis() - startWhen);
                        //Log.d(TAG, "info.presentationTimeUs : " + (info.presentationTimeUs / 1000) + " playTime: " + (System.currentTimeMillis() - startWhen) + " sleepTime : " + sleepTime);
                        if (sleepTime > 0) Thread.sleep(sleepTime);
                    } catch (InterruptedException e) {
                        e.printStackTrace();
                    }


                    if (frameIndex <= 50) {
                        outputBuffer = mediaCodec.getOutputBuffer(outputIndex);
                        outputBuffer.position(outputInfo.offset);
                        outputBuffer.limit(outputInfo.offset + outputInfo.size);
                        byte[] data = new byte[outputBuffer.remaining()];
                        // byteBuffer数据放入data
                        outputBuffer.get(data);
                        Log.d(TAG, "data: " + data);
                        // 输出的一帧保存为本地的一张图片
                        outputFrameAsPic(data, frameIndex++);
                    }

                    //对outputbuffer的处理完后，调用这个函数把buffer重新返回给codec类。
                    //调用这个api之后，SurfaceView才有图像
                    mediaCodec.releaseOutputBuffer(outputIndex, true /* Surface init */);
                }
            }


        } catch (Exception e) {
            e.printStackTrace();
        } finally {
            if (mediaCodec != null) {
                mediaCodec.stop();
                mediaCodec.release();
            }
            if (mediaExtractor != null) {
                mediaExtractor.release();
            }
        }
    };


    /**
     * 根据视频大小改变SurfaceView大小
     */
    private void changeVideoSize() {

        int surfaceWidth = mediacodecView.getMeasuredWidth();
        int surfaceHeight = mediacodecView.getMeasuredHeight();
        //根据视频尺寸去计算->视频可以在sufaceView中放大的最大倍数。
        Double maxSize;

        if (getResources().getConfiguration().orientation == ActivityInfo.SCREEN_ORIENTATION_PORTRAIT) {
            //竖屏模式下按视频宽度计算放大倍数值
            maxSize = (double) Math.max(videoWidth / surfaceWidth, videoHeight / surfaceHeight);
        } else {
            //横屏模式下按视频高度计算放大倍数值
            maxSize = (double) Math.max(videoWidth / surfaceHeight, videoHeight / surfaceWidth);
        }

        //视频宽高分别/最大倍数值 计算出放大后的视频尺寸
        videoWidth = (int) Math.ceil(videoWidth / maxSize);
        videoHeight = (int) Math.ceil(videoHeight / maxSize);

        //将计算出的视频尺寸设置到surfaceView 让视频自动填充。
        mediacodecView.setLayoutParams(new LinearLayout.LayoutParams(videoWidth, videoHeight));
    }


    /**
     * 将每一帧存为图片
     *
     * @param data
     * @param frameIndex
     */
    private void outputFrameAsPic(byte[] data, int frameIndex) {

        Log.d(TAG, "outputBuffer i：" + frameIndex);
        YuvImage yuvImage = new YuvImage(data, ImageFormat.NV21, videoWidth, videoHeight, null);
        ByteArrayOutputStream baos = new ByteArrayOutputStream();
        //将yuv转化为jpeg
        yuvImage.compressToJpeg(new Rect(0, 0, videoWidth, videoHeight), 100, baos);
        byte[] jdata = baos.toByteArray();//rgb
        Bitmap bmp = BitmapFactory.decodeByteArray(jdata, 0, jdata.length);
        if (bmp != null) {
            try {
                File parent = new File(Environment.getExternalStorageDirectory(), "Atestvideo/image");
                if (!parent.exists()) {
                    parent.mkdirs();
                }

                File myCaptureFile = new File(parent.getAbsolutePath(), String.format("img%s.png", frameIndex));
                if (!myCaptureFile.exists()) {
                    myCaptureFile.createNewFile();
                }
                BufferedOutputStream bos = new BufferedOutputStream(new FileOutputStream(myCaptureFile));
                bmp.compress(Bitmap.CompressFormat.JPEG, 80, bos);
                Log.d(TAG, "bmp.compress myCaptureFile:" + myCaptureFile.getAbsolutePath());
                bos.flush();
                bos.close();
            } catch (Exception e) {
                e.printStackTrace();
                Log.d(TAG, "outputFrameAsPic Exception:" + e);
            }
        }
    }


    @Override
    public void surfaceCreated(@NonNull SurfaceHolder holder) {
        initMedia();
        isDecoding = true;
        decodeThread = new Thread(decodeVideoRunnable);
        decodeThread.start();
    }

    @Override
    public void surfaceChanged(@NonNull SurfaceHolder holder, int format, int width, int height) {

    }

    @Override
    public void surfaceDestroyed(@NonNull SurfaceHolder holder) {
        isDecoding = false;
    }
}