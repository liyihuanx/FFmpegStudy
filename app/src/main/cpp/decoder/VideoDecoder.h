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
        LOGD("VideoDecoder")
        onCreate(url, AVMEDIA_TYPE_VIDEO);
    }

    virtual ~VideoDecoder() {
        LOGD("~VideoDecoder")
        onDestroy();
    }

    void SetVideoRender(BaseRender *render) {
        videoRender = render;
    }

private:
    virtual void initDecoderEnvironment();

    virtual void onFrameAvailable(AVFrame *frame);


private:
    int video_height;
    int video_width;

    AVFrame *frame_rgb = nullptr;
    uint8_t *frame_rgb_buffer = nullptr;
    SwsContext *videoSwsCtx = nullptr;
    BaseRender *videoRender = nullptr;
};


#endif //FFMPEGSTUDY_VIDEODECODER_H
