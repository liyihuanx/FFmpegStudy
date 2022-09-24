//
// Created by k1 on 2022/9/23.
//

#ifndef FFMPEGSTUDY_BASECHANNEL_H
#define FFMPEGSTUDY_BASECHANNEL_H

#endif //FFMPEGSTUDY_BASECHANNEL_H

extern "C" {
#include <libavformat/avformat.h>
#include <libavutil/imgutils.h>
#include "libavcodec/avcodec.h"
#include "libswscale/swscale.h"
#include <libswresample/swresample.h>
};
#include "log4c.h"

class BaseChannel {

public:
    AVCodecContext *av_codec_ctx = nullptr;

    AVPacket *packet = nullptr;
    AVFrame *frame = nullptr;


public:
    BaseChannel() {

    }

    virtual ~BaseChannel() {
        LOGD("BaseChannel的析构函数");
    }
};