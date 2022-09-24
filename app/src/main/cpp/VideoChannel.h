//
// Created by k1 on 2022/9/23.
//

#ifndef FFMPEGSTUDY_VIDEOCHANNEL_H
#define FFMPEGSTUDY_VIDEOCHANNEL_H

#include "BaseChannel.h"

class VideoChannel : public BaseChannel {

public:
    AVCodec *video_codec = nullptr;
    SwsContext *video_sws_ctx = nullptr;

    AVFrame *video_translate_frame = nullptr;

    int video_index = -1;

public:
    VideoChannel();
    ~VideoChannel();

    int video_init();
    int video_decode();
};


#endif //FFMPEGSTUDY_VIDEOCHANNEL_H
