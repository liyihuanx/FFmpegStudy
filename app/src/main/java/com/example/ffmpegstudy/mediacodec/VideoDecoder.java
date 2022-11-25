package com.example.ffmpegstudy.mediacodec;

import android.graphics.Bitmap;
import android.graphics.ImageFormat;
import android.graphics.Rect;
import android.media.Image;
import android.media.MediaCodec;
import android.media.MediaCodecInfo;
import android.media.MediaCodecList;
import android.media.MediaExtractor;
import android.media.MediaFormat;
import android.media.MediaMuxer;
import android.os.Environment;
import android.os.FileUtils;
import android.text.TextUtils;
import android.util.Log;

import com.example.ffmpegstudy.Constant;
import com.example.ffmpegstudy.camera.CameraUtil;

import java.io.BufferedOutputStream;
import java.io.FileOutputStream;
import java.io.IOException;
import java.nio.ByteBuffer;

/**
 * @ClassName: VideoEncode
 * @Description: java类作用描述
 * @Author: liyihuan
 * @Date: 2022/11/23 21:54
 */
class VideoDecoder {
    private static final String TAG = "VideoDecoder";
    public final static int Y = 0;
    public final static int U = 1;
    public final static int V = 2;

    public final static int COLOR_FORMAT_I420 = 1;
    public final static int COLOR_FORMAT_NV21 = 2;
    public final static int COLOR_FORMAT_NV12 = 3;


//    private final String inputFile = Environment.getExternalStorageDirectory() + "/Atestvideo/test.mp4";
//    private final String outputFile = Environment.getExternalStorageDirectory() + "/Atestvideo/mediacodecout.mp4";
//    private final String outputYuvFile = Environment.getExternalStorageDirectory() + "/Atestvideo/mediacodecout.yuv";


    private final String inputFile = "/data/data/com.example.ffmpegstudy/cache/test.mp4";
    private final String outputFile = "/data/data/com.example.ffmpegstudy/cache/mediacodecout.mp4";
    private final String outputYuvFile = "/data/data/com.example.ffmpegstudy/cache/mediacodecout.yuv";


//    private final String inputFile = Constant.getTestMp4FilePath();
//    private final String outputYuvFile = Constant.getTestFilePath("mediacodecout.yuv");
//    private final String outputFile = Constant.getTestFilePath("mediacodecout.mp4");
//    private final String outputH264File = Constant.getTestFilePath("mediacodecout.h264");


    private MediaExtractor mediaExtractor;
    private MediaFormat decodeMediaFormat;
    private MediaCodec decoder;
    private Thread decodeTread;

    private boolean isStop = false;

    private PreviewCallback previewCallback;

    private int videoWidth;
    private int videoHeight;
    private long totalTime;
    private int frameRate;

    private static final int DECODE_COLOR_FORMAT = MediaCodecInfo.CodecCapabilities.COLOR_FormatYUV420Flexible;
    private static final long DEFAULT_TIMEOUT_US = 10000;


    // 编码器
    private VideoEncoder videoEncoder;

    public interface PreviewCallback {
        void info(int width, int height, int fps);

        void getBitmap(Bitmap bitmap);

        void progress(int progress);

        void onDecode(byte[] yuv, int width, int height, int frameCount, long presentationTimeUs);

        void onFinish();

    }

    public void setPreviewCallback(PreviewCallback previewCallback) {
        this.previewCallback = previewCallback;
    }


    public VideoDecoder(){
        init();
    }

    private void init() {
        mediaExtractor = new MediaExtractor();
        try {
            mediaExtractor.setDataSource(inputFile);
            // 选择视频轨
            int trackIndex = selectVideoTrackIndex();
            if (trackIndex < 0) {
                Log.e(TAG, "No video track found in " + inputFile);
                return;
            }
            //
            decodeMediaFormat = mediaExtractor.getTrackFormat(trackIndex);

            String mime = decodeMediaFormat.getString(MediaFormat.KEY_MIME);
            Log.d(TAG, "mediaFormat.getString(MediaFormat.KEY_MIME) mime= " + mime);

            // 一些视频数据
            initVideoInfo();

            // MediaCodecInfo.CodecCapabilities 存储了编码器所有支持的颜色格式
            //     原始数据                  编码器
            // NV12(YUV420sp) ———> COLOR_FormatYUV420PackedSemiPlanar
            // NV21           ———> COLOR_FormatYUV420SemiPlanar
            // YV12(I420)     ———> COLOR_FormatYUV420Planar
//            if (isColorFormatSupported(DECODE_COLOR_FORMAT, decoder.getCodecInfo().getCapabilitiesForType(mime))) {
//                // MediaFormat.KEY_COLOR_FORMAT: 指明video编码器的颜色格式
//                decodeMediaFormat.setInteger(MediaFormat.KEY_COLOR_FORMAT, DECODE_COLOR_FORMAT);
//            }
            decodeMediaFormat.setInteger(MediaFormat.KEY_COLOR_FORMAT, MediaCodecInfo.CodecCapabilities.COLOR_FormatYUV420Planar);
            // 创建解码器
            decoder = MediaCodec.createDecoderByType(mime);
            decoder.configure(decodeMediaFormat, null, null, 0);
            decoder.start();

            Log.d(TAG, "encodeMediaFormat::" + decodeMediaFormat.toString());

            videoEncoder = new VideoEncoder(videoWidth,videoHeight);


        } catch (IOException e) {
            e.printStackTrace();
        }
    }

    public void startDecode(){
        startDecodeThread();
    }

    public void release(){
        if (decoder != null) {
            decoder.stop();
            decoder.release();
            decoder = null;
        }
    }


    private void initVideoInfo() {
        // 宽，高，时长，帧率
        videoWidth = decodeMediaFormat.getInteger(MediaFormat.KEY_WIDTH);
        videoHeight = decodeMediaFormat.getInteger(MediaFormat.KEY_HEIGHT);
        totalTime = decodeMediaFormat.getLong(MediaFormat.KEY_DURATION);
        frameRate = decodeMediaFormat.getInteger(MediaFormat.KEY_FRAME_RATE);
        Log.d(TAG, "videoWidth: " + videoWidth + ", videoHeight: " + videoHeight + ", totalTime: " + totalTime + ", frameRate: " + frameRate);
        if (previewCallback != null) {
            previewCallback.info(videoWidth, videoHeight, frameRate);
        }

        // 根据MediaFormat的csd-0，csd-1中分析出配置帧的数据。H264中sps为csd-0，pps为csd-1
        ByteBuffer byteBuffer0 = decodeMediaFormat.getByteBuffer("csd-0");
        ByteBuffer byteBuffer1 = decodeMediaFormat.getByteBuffer("csd-1");
        StringBuilder sb0 = new StringBuilder();
        for (byte b : byteBuffer0.array()) {
            sb0.append(b).append(", ");
        }
        StringBuilder sb1 = new StringBuilder();
        for (byte b : byteBuffer1.array()) {
            sb1.append(b).append(", ");
        }
        Log.d(TAG, "csd-0: " + sb0);
        Log.d(TAG, "csd-1: " + sb1);
    }

    private void startDecodeThread() {
        if (decodeTread == null) {
            decodeTread = new Thread(this::decodeFrame);
        }
        decodeTread.start();
    }


    private void decodeFrame() {
        Log.d(TAG, "decodeFrame");
        MediaCodec.BufferInfo videoBufferInfo = new MediaCodec.BufferInfo();

        boolean isInputEOS = false;
        boolean isOutputEOS = false;
        int frameCount = 0;

        while (!isStop) {
            if (!isInputEOS) {
                int inputBufferIndex = decoder.dequeueInputBuffer(DEFAULT_TIMEOUT_US);
                if (inputBufferIndex >= 0) {
                    // 使用返回的inputBuffer的index得到一个ByteBuffer，可以放数据了
                    ByteBuffer inputBuffer = decoder.getInputBuffer(inputBufferIndex);
                    // 使用extractor往MediaCodec的InputBuffer里面写入数据，-1表示已全部读取完
                    int sampleSize = mediaExtractor.readSampleData(inputBuffer, 0);
                    if (sampleSize < 0) {
                        decoder.queueInputBuffer(inputBufferIndex, 0, 0, 0, MediaCodec.BUFFER_FLAG_END_OF_STREAM);
                        isInputEOS = true;
                    } else {
                        // 填充好的数据写入第inputBufferIndex个InputBuffer，分贝设置size和sampleTime，这里sampleTime不一定是顺序来的，所以需要缓冲区来调节顺序。
                        decoder.queueInputBuffer(inputBufferIndex, 0, sampleSize, mediaExtractor.getSampleTime(), 0);
                        // 在MediaExtractor执行完一次readSampleData方法后，需要调用advance()去跳到下一个sample，然后再次读取数据
                        mediaExtractor.advance();
                        isInputEOS = false;
                    }
                }
            }

            int outputBufferIndex = decoder.dequeueOutputBuffer(videoBufferInfo, DEFAULT_TIMEOUT_US);
            switch (outputBufferIndex) {
                case MediaCodec.INFO_OUTPUT_FORMAT_CHANGED:
                    Log.v(TAG, outputBufferIndex + " format changed");
                    break;
                case MediaCodec.INFO_TRY_AGAIN_LATER:
                    Log.v(TAG, outputBufferIndex + " 解码当前帧超时");
                    break;
                case MediaCodec.INFO_OUTPUT_BUFFERS_CHANGED:
                    Log.v(TAG, outputBufferIndex + " output buffers changed");
                    break;
                default:
                    long currTime = videoBufferInfo.presentationTimeUs;
                    Image image = decoder.getOutputImage(outputBufferIndex);
                    byte[] YUV = getDataFromImage(image, COLOR_FORMAT_I420);
                    writeToFile(YUV, outputYuvFile, true);

                    if (previewCallback != null) {
                        previewCallback.onDecode(YUV, videoWidth, videoHeight, frameCount++, currTime);
                    }

                    boolean end = ((videoBufferInfo.flags & MediaCodec.BUFFER_FLAG_END_OF_STREAM) != 0);
                    videoEncoder.encodeFrame(YUV, currTime, end);

//                    byte[] nv21bytes = BitmapUtil.I420Tonv21(i420bytes, width, height);
//                    Bitmap bitmap = BitmapUtil.getBitmapImageFromYUV(nv21bytes, width, height);
//                    previewCallback.getBitmap(bitmap);

//                    int progress = (int) (currTime * 100 / totalTime);
//                    previewCallback.progress(progress);
//
//                    Log.v(TAG, "progress::" + progress);

                    // 将该ByteBuffer释放掉，以供缓冲区的循环使用。
                    decoder.releaseOutputBuffer(outputBufferIndex, true);
                    break;
            }

            if ((videoBufferInfo.flags & MediaCodec.BUFFER_FLAG_END_OF_STREAM) != 0) {
                Log.v(TAG, "buffer stream end");
                if (previewCallback != null) {
                    previewCallback.onFinish();
                }
                break;
            }

        } // while end


        videoEncoder.release();

        mediaExtractor.release();
        decoder.stop();
        decoder.release();
    }

    private int encodeVideoTrackIndex = -1;


    private byte[] getDataFromImage(Image image, int colorFormat) {
        // 指定有效区域
        Rect crop = image.getCropRect();
        int format = image.getFormat();
        int width = crop.width();
        int height = crop.height();

        // Y,U,V 分量
        Image.Plane[] planes = image.getPlanes();

        byte[] data = new byte[width * height * ImageFormat.getBitsPerPixel(format) / 8];
        byte[] rowData = new byte[planes[0].getRowStride()];
        Log.d(TAG, "colorFormat：" + colorFormat + ", crop width: " + crop.width() + ", height: " + crop.height() + ", date.length:" + data.length + ", rowData.length:" + rowData.length);

        // 偏移量
        int channelOffset = 0;
        // 步长
        int outputStride = 1;
        for (int i = 0; i < planes.length; i++) {
            switch (i) {
                case Y: {
                    channelOffset = 0;
                    outputStride = 1;
                    break;
                }
                case U: {
                    if (colorFormat == COLOR_FORMAT_I420) {
                        channelOffset = width * height;
                        outputStride = 1;
                    } else if (colorFormat == COLOR_FORMAT_NV21) {
                        channelOffset = width * height + 1;
                        outputStride = 2;
                    } else if (colorFormat == COLOR_FORMAT_NV12) {
                        channelOffset = width * height;
                        outputStride = 2;
                    }
                    break;
                }
                case V:
                    if (colorFormat == COLOR_FORMAT_I420) {
                        channelOffset = (int) (width * height * 1.25);
                        outputStride = 1;
                    } else if (colorFormat == COLOR_FORMAT_NV21) {
                        channelOffset = width * height;
                        outputStride = 2;
                    } else if (colorFormat == COLOR_FORMAT_NV12) {
                        channelOffset = width * height + 1;
                        outputStride = 2;
                    }
                    break;
                default:
            }

            // 获取当前分量
            ByteBuffer buffer = planes[i].getBuffer();
            int rowStride = planes[i].getRowStride();
            int pixelStride = planes[i].getPixelStride();
            Log.v(TAG, "pixelStride " + pixelStride + ", rowStride " + rowStride + ", width " + width + ", height " + height + ", buffer size " + buffer.remaining());

            // 不是Y分量，右移一位 == 宽高都缩小一倍
            int shift = (i == 0) ? 0 : 1;
            int w = width >> shift;
            int h = height >> shift;
            //
            buffer.position(rowStride * (crop.top >> shift) + pixelStride * (crop.left >> shift));

            for (int row = 0; row < h; row++) {
                int length;
                if (pixelStride == 1 && outputStride == 1) {
                    length = w;
                    buffer.get(data, channelOffset, length);
                    channelOffset += length;
                } else {
                    // 计算UV分量的长度
                    length = (w - 1) * pixelStride + 1;
                    // 获取UV分量的数据
                    buffer.get(rowData, 0, length);
                    // 单独取出某个分量
                    for (int col = 0; col < w; col++) {
                        data[channelOffset] = rowData[col * pixelStride];
                        channelOffset += outputStride;
                    }
                }
                if (row < h - 1) {
                    buffer.position(buffer.position() + rowStride - length);
                }
            }
        }

        return data;
    }


    private int selectVideoTrackIndex() {
        if (mediaExtractor != null) {
            for (int i = 0; i < mediaExtractor.getTrackCount(); i++) {
                MediaFormat trackFormat = mediaExtractor.getTrackFormat(i);
                String mime = trackFormat.getString(MediaFormat.KEY_MIME);
                if (mime.contains("video/")) {
                    // 选择后，只会返回此轨的信息
                    mediaExtractor.selectTrack(i);
                    return i;
                }
            }
        }
        return -1;
    }




    private boolean isColorFormatSupported(int colorFormat, MediaCodecInfo.CodecCapabilities caps) {
        for (int c : caps.colorFormats) {
            if (c == colorFormat) {
                return true;
            }
        }
        return false;
    }


    private boolean writeToFile(byte[] bytes, String fullfilename, boolean isAppend) {
        if (bytes == null || TextUtils.isEmpty(fullfilename)) {
            return false;
        }
        BufferedOutputStream out = null;
        try {
            out = new BufferedOutputStream(new FileOutputStream(fullfilename, isAppend));
            out.write(bytes);
        } catch (IOException ioe) {
            Log.e(TAG, "IOException : " + ioe.getMessage());
            return false;
        } finally {
            try {
                if (out != null) {
                    out.flush();
                    out.close();
                }
            } catch (IOException ioe) {
                Log.e(TAG, "Exception:" + ioe.getMessage());
                return false;
            }
        }
        return true;
    }

    // OMX.google开头的是软解码
    // OMX.开头的是硬解码
    private String getExpectedEncodeCodec(String expectedMimeType, int expectedColorFormat) {
        MediaCodecList allMediaCodecLists = new MediaCodecList(-1);
        for (MediaCodecInfo mediaCodecInfo : allMediaCodecLists.getCodecInfos()) {
            if (mediaCodecInfo.isEncoder()) {
                String[] supportTypes = mediaCodecInfo.getSupportedTypes();
                for (String supportType : supportTypes) {
                    if (supportType.equals(expectedMimeType)) {
                        Log.d(TAG, "编码器名称:" + mediaCodecInfo.getName() + "  " + supportType);
                        MediaCodecInfo.CodecCapabilities codecCapabilities = mediaCodecInfo.getCapabilitiesForType(expectedMimeType);
                        int[] colorFormats = codecCapabilities.colorFormats;
                        for (int colorFormat : colorFormats) {
                            if (colorFormat == expectedColorFormat) {
                                return mediaCodecInfo.getName();
                            }
                        }
                    }
                }
            }
        }
        return "";
    }

}
