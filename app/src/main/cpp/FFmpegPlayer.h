//
// Created by k1 on 2022/9/8.
//

#ifndef FFMPEGSTUDY_FFMPEGPLAYER_H
#define FFMPEGSTUDY_FFMPEGPLAYER_H

extern "C" {
#include <libavformat/avformat.h>
};

#include "log4c.h"

class FFmpegPlayer {
private:
    char *data_source = nullptr;

    // 上下文
    AVFormatContext *av_format_ctx = nullptr;
    AVCodecContext *av_codec_ctx = nullptr;

    AVCodec *av_codec = nullptr;

    int video_index = -1;
    int audio_index = -1;


    AVFrame *frame = nullptr;
    AVPacket *packet = nullptr;

public:
    FFmpegPlayer();

    ~FFmpegPlayer();

    void prepare(char *data);

    void start();

    void release();

};


#endif //FFMPEGSTUDY_FFMPEGPLAYER_H
