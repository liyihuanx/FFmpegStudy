//
// Created by leeyh on 2022/9/27.
//

#ifndef FFMPEGSTUDY_BASEDECODER_H
#define FFMPEGSTUDY_BASEDECODER_H

extern "C" {
#include <libavformat/avformat.h>
#include <libavutil/imgutils.h>
#include "libavcodec/avcodec.h"
#include "libswscale/swscale.h"
#include <libswresample/swresample.h>
};

#include "../log4c.h"

class BaseDecoder {

public:
    BaseDecoder();
    virtual ~BaseDecoder();


private:
    AVFormatContext *avFormatContext = nullptr;

    AVCodecContext *avCodecContext = nullptr;

    AVCodec *avCodec = nullptr;

    AVFrame *frame = nullptr;

    AVPacket *packet = nullptr;


    // 音视频数据
    char *data_source = nullptr;

    int video_stream = -1;
    int audio_stream = -1;

public:
    virtual void onCreate(char* url);
    virtual void onDestroy();

private:
    int initFFmpegCtx();
    int initDecoder();
    int startDecoder();
};


#endif //FFMPEGSTUDY_BASEDECODER_H
