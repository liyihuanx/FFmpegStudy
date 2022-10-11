//
// Created by leeyh on 2022/9/27.
//

#ifndef FFMPEGSTUDY_VIDEODECODER_H
#define FFMPEGSTUDY_VIDEODECODER_H


#include "BaseDecoder.h"
#include "../render/BaseRender.h"

class VideoDecoder : public BaseDecoder {

public:
    VideoDecoder(char *url) {
        onCreate(url, AVMEDIA_TYPE_AUDIO);
    }

    virtual ~VideoDecoder() {
        onDestroy();
    }

    virtual void initDecoderEnvironment();
    virtual void OnFrameAvailable(AVFrame *frame);

private:
    int video_height;
    int video_width;

    AVFrame *frame_rgb = nullptr;
    uint8_t *frame_rgb_buffer = nullptr;
    SwsContext *videoSwsCtx = nullptr;
    BaseRender *baseRender = nullptr;
};


#endif //FFMPEGSTUDY_VIDEODECODER_H
