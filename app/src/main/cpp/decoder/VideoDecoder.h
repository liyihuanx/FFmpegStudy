//
// Created by leeyh on 2022/9/27.
//

#ifndef FFMPEGSTUDY_VIDEODECODER_H
#define FFMPEGSTUDY_VIDEODECODER_H


#include "BaseDecoder.h"

class VideoDecoder : public BaseDecoder{

public:
    VideoDecoder(char *url) {
        onCreate(url,AVMEDIA_TYPE_AUDIO);
    }
    virtual ~VideoDecoder() {
        onDestroy();
    }

};


#endif //FFMPEGSTUDY_VIDEODECODER_H
