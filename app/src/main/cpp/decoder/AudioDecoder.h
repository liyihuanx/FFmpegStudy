//
// Created by leeyh on 2022/9/27.
//

#ifndef FFMPEGSTUDY_AUDIODECODER_H
#define FFMPEGSTUDY_AUDIODECODER_H

#include "BaseDecoder.h"

class AudioDecoder : public BaseDecoder{
public:
    AudioDecoder(char *url) {
        onCreate(url,AVMEDIA_TYPE_AUDIO);
    }
    virtual ~AudioDecoder() {
        onDestroy();
    }


    virtual int initDecoder();
};


#endif //FFMPEGSTUDY_AUDIODECODER_H
