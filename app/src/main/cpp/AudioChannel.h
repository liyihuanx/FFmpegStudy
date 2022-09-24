//
// Created by k1 on 2022/9/23.
//

#ifndef FFMPEGSTUDY_AUDIOCHANNEL_H
#define FFMPEGSTUDY_AUDIOCHANNEL_H

#include "BaseChannel.h"

class AudioChannel : public BaseChannel {
public:
    AVCodec *audio_codec = nullptr;
    SwsContext *audio_sws_ctx = nullptr;

    AVFrame *audio_translate_frame = nullptr;

    int audio_index = -1;

    // audio参数
    int out_channels;
    int out_sample_size;
    int out_sample_rate;
    int out_buffers_size;
    uint8_t *out_buffers = nullptr;
    SwrContext *swr_ctx = nullptr;

public:
    AudioChannel();
    ~AudioChannel();

    // audio context，codec等初始化
    int audio_init();
    // audio 解码
    int audio_decode();
};


#endif //FFMPEGSTUDY_AUDIOCHANNEL_H
