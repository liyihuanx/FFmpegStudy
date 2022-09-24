//
// Created by k1 on 2022/9/8.
//

#ifndef FFMPEGSTUDY_FFMPEGPLAYER_H
#define FFMPEGSTUDY_FFMPEGPLAYER_H

extern "C" {
#include <libavformat/avformat.h>
#include <libavutil/imgutils.h>
#include "libavcodec/avcodec.h"
#include "libswscale/swscale.h"
#include <libswresample/swresample.h>
};

#include "log4c.h"
#include <android/native_window_jni.h>
#include <SLES/OpenSLES.h>
#include <SLES/OpenSLES_Android.h>

class FFmpegPlayer {
private:
    char *data_source = nullptr;

    // 上下文
    AVFormatContext *av_format_ctx = nullptr;
    AVCodecContext *av_codec_ctx = nullptr;
    AVCodec *av_codec = nullptr;

    int video_index = -1;
    int audio_index = -1;


    AVCodecContext *audio_codec_ctx = nullptr;
    AVCodec *audio_codec = nullptr;

    int video_height = -1;
    int video_width = -1;

    ANativeWindow *native_window = nullptr;

    //引擎
    SLObjectItf engineObject = 0;
    // 引擎接口
    SLEngineItf engineInterface = 0;
    // 混音器
    SLObjectItf outputMixObject = 0;
    // 播放器
    SLObjectItf bqPlayerObject = 0;
    // 播放器接口
    SLPlayItf bqPlayerPlayInterface = 0;
    // 播放器队列接口
    SLAndroidSimpleBufferQueueItf bqPlayerBufferQueue = 0;

public:
    AVFrame *frame = nullptr;
    AVPacket *packet = nullptr;
    AVFrame *frame_rgb = nullptr;

    uint8_t *frame_rgb_buffer = nullptr;

    int out_channels;
    int out_sample_size;
    int out_sample_rate;
    int out_buffers_size;
    uint8_t *out_buffers = nullptr;
    SwrContext *swr_ctx = nullptr;


public:
    FFmpegPlayer();

    ~FFmpegPlayer();

    void prepare(char *data);

    void start();

    void release();

    void setWindow(ANativeWindow *native_window);

    void initOpenSLES();

};


#endif //FFMPEGSTUDY_FFMPEGPLAYER_H
