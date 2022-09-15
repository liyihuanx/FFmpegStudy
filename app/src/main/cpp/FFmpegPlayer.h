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

class FFmpegPlayer {
private:
    char *data_source = nullptr;

    // 上下文
    AVFormatContext *av_format_ctx = nullptr;
    AVCodecContext *av_codec_ctx = nullptr;

    AVCodec *av_codec = nullptr;

    SwsContext *sws_context = nullptr;

    int video_index = -1;
    int audio_index = -1;


    AVFrame *frame = nullptr;
    AVPacket *packet = nullptr;
    AVFrame *frame_rgb = nullptr;

    uint8_t *frame_rgb_buffer = nullptr;

    int video_height = -1;
    int video_width = -1;

    ANativeWindow *native_window = nullptr;

public:
    FFmpegPlayer();

    ~FFmpegPlayer();

    void prepare(char *data);

    void start();

    void release();

    void setWindow(ANativeWindow *native_window);
};


#endif //FFMPEGSTUDY_FFMPEGPLAYER_H
