package com.example.ffmpegstudy.mediacodec;

import android.media.MediaCodec;
import android.media.MediaCodecInfo;
import android.media.MediaCodecList;
import android.media.MediaFormat;
import android.media.MediaMuxer;
import android.os.FileUtils;
import android.text.TextUtils;
import android.util.Log;

import java.io.BufferedOutputStream;
import java.io.FileOutputStream;
import java.io.IOException;
import java.nio.ByteBuffer;

/**
 * @author liyihuan
 * @date 2022/11/25
 * @description
 */
public class VideoEncoder {

    private static final String TAG = "VideoEncoder";

    private MediaCodec encoder;
    private MediaFormat encodeMediaFormat;
    private MediaMuxer encodeMediaMuxer;

    private int encodeVideoTrackIndex = -1;

    private static final long DEFAULT_TIMEOUT_US = 10000;


    private final String outputFile = "/data/data/com.example.ffmpegstudy/cache/mediacodecout.mp4";
    private final String outputH264File = "/data/data/com.example.ffmpegstudy/cache/mediacodecout.h264";

    public VideoEncoder(int width, int height){
        init(width, height);
    }

    private void init(int width, int height) {
        try {
            int colorFormat = MediaCodecInfo.CodecCapabilities.COLOR_FormatYUV420Planar;
            String codecName = getExpectedEncodeCodec(MediaFormat.MIMETYPE_VIDEO_AVC, colorFormat);
            if (TextUtils.isEmpty(codecName)) {
                colorFormat = MediaCodecInfo.CodecCapabilities.COLOR_FormatYUV420SemiPlanar;
                codecName = getExpectedEncodeCodec(MediaFormat.MIMETYPE_VIDEO_AVC, colorFormat);
            }
            int encodeFrameRate = 20;
            encodeMediaFormat = MediaFormat.createVideoFormat(MediaFormat.MIMETYPE_VIDEO_AVC, width, height);
            // 设置编码的颜色格式
            encodeMediaFormat.setInteger(MediaFormat.KEY_COLOR_FORMAT, colorFormat);
            // biteRate
            encodeMediaFormat.setInteger(MediaFormat.KEY_BIT_RATE, 2500000);
            // 帧率
            encodeMediaFormat.setInteger(MediaFormat.KEY_FRAME_RATE, encodeFrameRate);
            // 设置I帧（关键帧）的间隔时间，单位秒
            encodeMediaFormat.setInteger(MediaFormat.KEY_I_FRAME_INTERVAL, 1);

            // 创建编码器、配置和启动
            encoder = MediaCodec.createByCodecName(codecName);
            encoder.configure(encodeMediaFormat, null, null, MediaCodec.CONFIGURE_FLAG_ENCODE);
            encoder.start();

            encodeMediaMuxer = new MediaMuxer(outputFile, MediaMuxer.OutputFormat.MUXER_OUTPUT_MPEG_4);

        } catch (IOException e) {
            e.printStackTrace();
        }

    }


    public void encodeFrame(byte[] yuvData, long presentationTimeUs, boolean isVideoEOS) {
        MediaCodec.BufferInfo encodeOutputBufferInfo = new MediaCodec.BufferInfo();
        MediaCodec.BufferInfo muxerOutputBufferInfo = new MediaCodec.BufferInfo();

        int inputBufferIndex = encoder.dequeueInputBuffer(-1);
        if (inputBufferIndex >= 0) {
            ByteBuffer inputBuffer = encoder.getInputBuffer(inputBufferIndex);
            // 放入数据
            inputBuffer.put(yuvData);
            // 将buffer压入解码队列中
            encoder.queueInputBuffer(inputBufferIndex, 0, yuvData.length, presentationTimeUs, 0);
        }

        if ((encodeOutputBufferInfo.flags & MediaCodec.BUFFER_FLAG_END_OF_STREAM) != 0) {
            Log.d(TAG, " encode  buffer stream end");
        }

        int outputBufferIndex = encoder.dequeueOutputBuffer(encodeOutputBufferInfo, -1);
        switch (outputBufferIndex) {
            case MediaCodec.INFO_OUTPUT_FORMAT_CHANGED:
                MediaFormat newFormat = encoder.getOutputFormat();
//                byte[] header_sps = {0, 0, 0, 1, 39, 100, 0, 31, -84, 86, -64, -120, 30, 105, -88, 8, 8, 8, 16};
//                byte[] header_pps = {0, 0, 0, 1, 40, -18, 60, -80};
//                encodeMediaFormat.setByteBuffer("csd-0", ByteBuffer.wrap(header_sps));
//                encodeMediaFormat.setByteBuffer("csd-1", ByteBuffer.wrap(header_pps));
//                encodeVideoTrackIndex = mMediaMuxer.addTrack(encodeMediaFormat);
                encodeVideoTrackIndex = encodeMediaMuxer.addTrack(newFormat);
                encodeMediaMuxer.start();
                break;
            case MediaCodec.INFO_TRY_AGAIN_LATER:
                break;
            default:
                ByteBuffer outputBuffer = encoder.getOutputBuffer(outputBufferIndex);
                byte[] outData = new byte[encodeOutputBufferInfo.size];
                outputBuffer.get(outData);
                writeToFile(outData, outputH264File, true);

                MediaFormat outputFormat = encoder.getOutputFormat(outputBufferIndex);
                Log.d(TAG, "encodeMediaFormat::" + encodeMediaFormat.toString());
                Log.d(TAG, "outputFormat::" + outputFormat.toString());

                muxerOutputBufferInfo.offset = 0;
                muxerOutputBufferInfo.size = encodeOutputBufferInfo.size;
                muxerOutputBufferInfo.flags = isVideoEOS ? MediaCodec.BUFFER_FLAG_END_OF_STREAM : MediaCodec.BUFFER_FLAG_KEY_FRAME;
                muxerOutputBufferInfo.presentationTimeUs = presentationTimeUs;
                Log.d(TAG, "presentationTimeUs::" + presentationTimeUs + " size::" + encodeOutputBufferInfo.size + "  isVideoEOS:" + isVideoEOS);
                encodeMediaMuxer.writeSampleData(encodeVideoTrackIndex, outputBuffer, encodeOutputBufferInfo);

                encoder.releaseOutputBuffer(outputBufferIndex, false);
                break;
        }

    }

    public void release(){
        // 一定要关闭，不然生成的MP4无法播放
        encodeMediaMuxer.stop();
        encodeMediaMuxer.release();
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
}
