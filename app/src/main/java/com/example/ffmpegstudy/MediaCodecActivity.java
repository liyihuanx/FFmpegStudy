package com.example.ffmpegstudy;

import androidx.annotation.NonNull;
import androidx.appcompat.app.AppCompatActivity;

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

import java.io.IOException;
import java.nio.ByteBuffer;

public class MediaCodecActivity extends AppCompatActivity implements SurfaceHolder.Callback {

    private static final String TAG = "MediaCodecActivity";

    private SurfaceView mediacodecView;
//    private SurfaceHolder mediacodecHolder;

    private MediaCodec mediaCodec;
    private MediaExtractor mediaExtractor;
    private String videoUrl = "/data/data/com.example.ffmpegstudy/cache/testvideo1.mp4";


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
                if (outputIndex >= 0) {
                    switch (outputIndex) {
                        case MediaCodec.INFO_OUTPUT_BUFFERS_CHANGED: {
                            Log.d(TAG, "INFO_OUTPUT_BUFFERS_CHANGED");
                            mediaCodec.getOutputBuffers();
                            break;
                        }
                        default:
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
                            //对outputbuffer的处理完后，调用这个函数把buffer重新返回给codec类。
                            //调用这个api之后，SurfaceView才有图像
                            mediaCodec.releaseOutputBuffer(outputIndex, true /* Surface init */);
                            break;
                    }
                }
            }


        } catch (Exception e) {
            e.printStackTrace();
        } finally {
//            mediaCodec.stop();
//            mediaCodec.release();
//            mediaExtractor.release();
        }
    };

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